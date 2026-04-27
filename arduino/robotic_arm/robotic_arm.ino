/*
 * 6-DOF Robotic Arm Control System
 *
 * Main firmware for an Arduino Uno based robotic arm driven by a Wii Nunchuck.
 * This sketch manages the runtime control loop, serial diagnostics, demo mode,
 * homing behavior, logical joint limits, and servo-aware output calibration.
 *
 * Project: robotic-arm-control
 * Author: Viktor
 * Hardware target: Arduino Uno
 */

#include <Servo.h>
#include <Wire.h>
#include <Arduino.h>
#define USE_WIICHUCK_LIB 0

#if USE_WIICHUCK_LIB
#include <WiiChuck.h>
Accessory nunchuck;
#endif

const uint8_t SERVO_PINS[6] = {3, 5, 6, 9, 10, 11};

enum Joint
{
  BASE = 0,
  SHOULDER,
  ELBOW,
  WRIST_ROLL,
  WRIST_PITCH,
  GRIPPER
};

Servo servos[6];

uint8_t currentAngle[6] = {90, 130, 90, 90, 90, 60};
uint8_t targetAngle[6] = {90, 130, 90, 90, 90, 60};

const uint8_t minAngle[6] = {0, 100, 0, 0, 20, 15};
const uint8_t maxAngle[6] = {180, 160, 160, 180, 160, 75};
const uint8_t homeAngle[6] = {90, 130, 90, 90, 90, 60};
// Reverse a joint here if the servo is mounted opposite to the logical angle.
const bool servoReversed[6] = {false, false, false, false, true, false};

// Calibration and tuning values:
// - minAngle/maxAngle/homeAngle define safe logical limits and the default pose
// - servoReversed applies mechanical inversion only when writing to the servo
// - WRIST_FILTER_ALPHA and WRIST_TARGET_DEADBAND reduce wrist jitter from the
//   Nunchuck accelerometer
// - GRIPPER_SPEED_PER_SEC controls how fast the gripper opens/closes while
//   C or Z is held
// - JOINT_SMOOTH_STEP keeps shoulder/elbow motion smoother than raw step changes
const int BASE_STOP_COMMAND = 90;
const int BASE_MIN_COMMAND = 70;
const int BASE_MAX_COMMAND = 110;
const float WRIST_FILTER_ALPHA = 0.14f;
const int WRIST_TARGET_DEADBAND = 1;
const float GRIPPER_SPEED_PER_SEC = 35.0f;

const int SMOOTH_STEP = 2;
const uint8_t JOINT_SMOOTH_STEP[6] = {0, SMOOTH_STEP, SMOOTH_STEP, 1, 1, SMOOTH_STEP};
const unsigned long CONTROL_INTERVAL_MS = 20;
const bool STARTUP_SELF_CHECK_ENABLED = true;
const unsigned long STARTUP_NUNCHUCK_PROBE_ATTEMPTS = 3;
const uint8_t MAX_RECORD_FRAMES = 48;
const unsigned long RECORD_INTERVAL_MS = 200;
unsigned long lastControlMs = 0;

// ---------------------------------------------------------------------------
// Nunchuck state
// ---------------------------------------------------------------------------
uint8_t joyX = 128;
uint8_t joyY = 128;
bool btnC = false;
bool btnZ = false;

// 10-bit accelerometer values. Neutral/flat ≈ 512 on all axes.
int accelX = 512;
int accelY = 512;
int accelZ = 512;

// ---------------------------------------------------------------------------
// Nunchuck layer system
//
//  Layer A (0): joy-Y → SHOULDER
//  Layer B (1): joy-Y → ELBOW
//  Both layers:
//    joy-X           → BASE speed (continuous rotation)
//    tilt X (accelX) → WRIST_ROLL
//    tilt Y (accelY) → WRIST_PITCH
//    C held >350ms   → GRIPPER open
//    Z held          → GRIPPER close
//    C tap <350ms    → switch layer A/B  (gripper NOT triggered on tap)
//    C+Z 0.7s hold   → toggle demo mode
//    C+Z 1.8s hold   → home position + back to nunchuck mode
// ---------------------------------------------------------------------------
uint8_t nunchuckLayer = 0;
bool cPrev = false;
unsigned long cPressMs = 0;
bool cTapValid = false;
const unsigned long C_TAP_MAX_MS = 350;

// C+Z combo state
bool comboPrev = false;
unsigned long comboStartMs = 0;
bool comboActionDone = false;
const unsigned long DEMO_TOGGLE_HOLD_MS = 700;
const unsigned long HOME_HOLD_MS = 1800;

enum ControlMode
{
  MODE_NUNCHUCK = 0,
  MODE_SERIAL,
  MODE_DEMO,
  MODE_REPLAY
};
ControlMode controlMode = MODE_NUNCHUCK;
ControlMode replayReturnMode = MODE_NUNCHUCK;
uint8_t selectedJoint = BASE;

uint8_t recordedFrames[MAX_RECORD_FRAMES][6];
uint8_t recordedFrameCount = 0;
uint8_t replayIndex = 0;
bool isRecording = false;
unsigned long lastRecordMs = 0;
unsigned long lastReplayMs = 0;

char serialBuffer[32];
uint8_t serialLen = 0;

// Forward declarations
void printHelp();
void printStatus();
void handleSerialInput();
void processSerialLine(char *line);
void jogSelectedJoint(int delta);
int parseJointToken(const char *token);
bool tokenEquals(const char *a, const char *b);
void printJointName(uint8_t joint);
void printModeName();
void printCompactStatus();
void printJointSummary(uint8_t joint);
void printPinMap();
bool isContinuousJoint(uint8_t joint);
void printRecordingStatus();
bool hasValidHomeConfiguration();
bool hasUniqueServoPins();
bool probeNunchuck(uint8_t attempts);
void runStartupSelfCheck();
void startRecording();
void stopRecording();
void clearRecording();
void captureRecordFrame(bool forceCapture);
void startReplay();
void replayControl();
void loadRecordedFrame(uint8_t frameIndex);

// ---------------------------------------------------------------------------
// Nunchuck I/O
// ---------------------------------------------------------------------------

void nunchuckWrite(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(0x52);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void initNunchuck()
{
#if USE_WIICHUCK_LIB
  nunchuck.begin();
  if (nunchuck.type == UnknownChuck)
    nunchuck.type = NUNCHUCK;
  delay(50);
#else
  Wire.begin();
  delay(50);
  nunchuckWrite(0xF0, 0x55);
  delay(1);
  nunchuckWrite(0xFB, 0x00);
  delay(1);
#endif
}

bool readNunchuck()
{
#if USE_WIICHUCK_LIB
  if (!nunchuck.readData())
    return false;
  joyX = nunchuck.getJoyX();
  joyY = nunchuck.getJoyY();
  btnC = nunchuck.getButtonC();
  btnZ = nunchuck.getButtonZ();
  // Uncomment if your WiiChuck version exposes accel (adjust range in manualControl if needed):
  // accelX = nunchuck.getAccelX();
  // accelY = nunchuck.getAccelY();
  // accelZ = nunchuck.getAccelZ();
  return true;

#else
  uint8_t data[6] = {0};
  Wire.beginTransmission(0x52);
  Wire.write((uint8_t)0x00);
  if (Wire.endTransmission() != 0)
    return false;
  delayMicroseconds(500);
  if (Wire.requestFrom(0x52, 6) != 6)
    return false;
  for (int i = 0; i < 6; i++)
    data[i] = Wire.read();

  joyX = data[0];
  joyY = data[1];

  // Reconstruct 10-bit accel: 8 MSBs in data[2..4], 2 LSBs packed in data[5]
  accelX = (data[2] << 2) | ((data[5] >> 2) & 0x03);
  accelY = (data[3] << 2) | ((data[5] >> 4) & 0x03);
  accelZ = (data[4] << 2) | ((data[5] >> 6) & 0x03);

  // Buttons are active-low in data[5] bits 0 and 1
  btnZ = !(data[5] & 0x01);
  btnC = !((data[5] >> 1) & 0x01);
  return true;
#endif
}

bool probeNunchuck(uint8_t attempts)
{
  for (uint8_t attempt = 0; attempt < attempts; attempt++)
  {
    if (readNunchuck())
      return true;
    delay(5);
  }
  return false;
}

// ---------------------------------------------------------------------------
// Joint helpers
// ---------------------------------------------------------------------------

bool hasValidHomeConfiguration()
{
  for (int i = 0; i < 6; i++)
  {
    if (isContinuousJoint(i))
      continue;
    if (homeAngle[i] < minAngle[i] || homeAngle[i] > maxAngle[i])
      return false;
  }
  return true;
}

bool hasUniqueServoPins()
{
  for (int i = 0; i < 6; i++)
  {
    for (int j = i + 1; j < 6; j++)
    {
      if (SERVO_PINS[i] == SERVO_PINS[j])
        return false;
    }
  }
  return true;
}

int mapJoystickToDelta(int value, int deadzone, int maxDelta)
{
  int centered = value - 128;
  if (abs(centered) < deadzone)
    return 0;
  return (int)map(centered, -128, 127, -maxDelta, maxDelta);
}

void applyJointLimits()
{
  for (int i = 0; i < 6; i++)
  {
    if (isContinuousJoint(i))
      targetAngle[i] = constrain(targetAngle[i], BASE_MIN_COMMAND, BASE_MAX_COMMAND);
    else
      targetAngle[i] = constrain(targetAngle[i], minAngle[i], maxAngle[i]);
  }
}

void setJointTarget(uint8_t joint, int angle)
{
  if (joint >= 6)
    return;
  targetAngle[joint] = angle;
  applyJointLimits();
}

bool isContinuousJoint(uint8_t joint) { return joint == BASE; }

// Keep the rest of the code in logical angles and only invert the final servo
// command when the physical mounting requires it.
uint8_t toServoCommand(uint8_t joint, uint8_t logicalAngle)
{
  if (joint >= 6 || !servoReversed[joint])
    return logicalAngle;
  return 180 - logicalAngle;
}

// Base is a continuous-rotation command, so it is written directly. The other
// joints are rate-limited toward their targets, with the wrist moving in smaller
// steps than the shoulder/elbow for finer control.
void smoothMoveStep()
{
  for (int i = 0; i < 6; i++)
  {
    if (isContinuousJoint(i))
    {
      currentAngle[i] = targetAngle[i];
    }
    else
    {
      int step = JOINT_SMOOTH_STEP[i];
      int diff = targetAngle[i] - currentAngle[i];
      if (diff > step)
        currentAngle[i] += step;
      else if (diff < -step)
        currentAngle[i] -= step;
      else
        currentAngle[i] = targetAngle[i];
    }
    servos[i].write(toServoCommand(i, currentAngle[i]));
  }
}

void setHomePosition()
{
  for (int i = 0; i < 6; i++)
    targetAngle[i] = isContinuousJoint(i) ? BASE_STOP_COMMAND : homeAngle[i];
}

void printRecordingStatus()
{
  Serial.print(F("Record: "));
  Serial.print(isRecording ? F("on") : F("off"));
  Serial.print(F(" | frames="));
  Serial.print((int)recordedFrameCount);
  Serial.print(F("/"));
  Serial.print((int)MAX_RECORD_FRAMES);
  if (controlMode == MODE_REPLAY)
  {
    Serial.print(F(" | replay="));
    Serial.print((int)(replayIndex + 1));
    Serial.print(F("/"));
    Serial.print((int)recordedFrameCount);
  }
  Serial.println();
}

void clearRecording()
{
  isRecording = false;
  recordedFrameCount = 0;
  replayIndex = 0;
  lastRecordMs = 0;
  lastReplayMs = 0;
}

void captureRecordFrame(bool forceCapture)
{
  if (!isRecording)
    return;

  unsigned long now = millis();
  if (!forceCapture && lastRecordMs != 0 && now - lastRecordMs < RECORD_INTERVAL_MS)
    return;

  if (recordedFrameCount >= MAX_RECORD_FRAMES)
  {
    isRecording = false;
    Serial.println(F("[record] buffer full, recording stopped."));
    return;
  }

  for (int i = 0; i < 6; i++)
    recordedFrames[recordedFrameCount][i] = targetAngle[i];

  recordedFrameCount++;
  lastRecordMs = now;

  if (recordedFrameCount >= MAX_RECORD_FRAMES)
  {
    isRecording = false;
    Serial.println(F("[record] buffer full, recording stopped."));
  }
}

void startRecording()
{
  if (controlMode == MODE_REPLAY)
  {
    Serial.println(F("stop replay before recording"));
    return;
  }

  isRecording = true;
  lastRecordMs = 0;
  captureRecordFrame(true);
  printRecordingStatus();
}

void stopRecording()
{
  isRecording = false;
  printRecordingStatus();
}

void loadRecordedFrame(uint8_t frameIndex)
{
  if (frameIndex >= recordedFrameCount)
    return;
  for (int i = 0; i < 6; i++)
    targetAngle[i] = recordedFrames[frameIndex][i];
  applyJointLimits();
}

void startReplay()
{
  if (recordedFrameCount == 0)
  {
    Serial.println(F("no recorded frames"));
    return;
  }

  isRecording = false;
  replayReturnMode = (controlMode == MODE_REPLAY) ? MODE_NUNCHUCK : controlMode;
  controlMode = MODE_REPLAY;
  replayIndex = 0;
  lastReplayMs = 0;
  loadRecordedFrame(replayIndex);
  printRecordingStatus();
}

void replayControl()
{
  if (recordedFrameCount == 0)
  {
    controlMode = replayReturnMode;
    return;
  }

  unsigned long now = millis();
  if (lastReplayMs == 0)
  {
    loadRecordedFrame(replayIndex);
    lastReplayMs = now;
    return;
  }

  if (now - lastReplayMs < RECORD_INTERVAL_MS)
    return;

  lastReplayMs = now;
  if (replayIndex + 1 >= recordedFrameCount)
  {
    controlMode = replayReturnMode;
    Serial.println(F("[replay] complete"));
    return;
  }

  replayIndex++;
  loadRecordedFrame(replayIndex);
}

void runStartupSelfCheck()
{
  if (!STARTUP_SELF_CHECK_ENABLED)
    return;

  bool pinsOk = hasUniqueServoPins();
  bool homesOk = hasValidHomeConfiguration();
  bool nunchuckOk = probeNunchuck(STARTUP_NUNCHUCK_PROBE_ATTEMPTS);

  Serial.println(F("[self-check] startup diagnostics"));
  Serial.print(F("  servo pin map: "));
  Serial.println(pinsOk ? F("ok") : F("duplicate pin detected"));
  Serial.print(F("  home limits:   "));
  Serial.println(homesOk ? F("ok") : F("invalid home angle"));
  Serial.print(F("  nunchuck I2C:  "));
  Serial.println(nunchuckOk ? F("detected") : F("not detected"));
  Serial.println(F("[self-check] done"));
}

// ---------------------------------------------------------------------------
// Serial print helpers
// ---------------------------------------------------------------------------

void printHelp()
{
  Serial.println(F("--- Commands ---"));
  Serial.println(F("  h/help       show this help"));
  Serial.println(F("  p            full status"));
  Serial.println(F("  pins         servo wiring"));
  Serial.println(F("  selfcheck    rerun startup diagnostics"));
  Serial.println(F("  mode s|n|d   serial / nunchuck / demo"));
  Serial.println(F("  sel <0-5>    select joint"));
  Serial.println(F("  set <j> <a>  set joint angle"));
  Serial.println(F("  step <j> <d> add delta to joint"));
  Serial.println(F("  rec start    start movement recording"));
  Serial.println(F("  rec stop     stop movement recording"));
  Serial.println(F("  rec clear    erase recorded frames"));
  Serial.println(F("  rec status   show record/replay status"));
  Serial.println(F("  play         replay recorded movement"));
  Serial.println(F("  stop         stop replay / recording"));
  Serial.println(F("--- Serial keys ---"));
  Serial.println(F("  1-6 select | a/z jog -/+ | m/n/d mode"));
  Serial.println(F("--- Nunchuck (6 joints) ---"));
  Serial.println(F("  joy X        -> BASE"));
  Serial.println(F("  joy Y        -> SHOULDER (layer A) or ELBOW (layer B)"));
  Serial.println(F("  tilt X       -> WRIST_ROLL"));
  Serial.println(F("  tilt Y       -> WRIST_PITCH"));
  Serial.println(F("  C tap <350ms -> switch layer A/B"));
  Serial.println(F("  C held       -> GRIPPER open"));
  Serial.println(F("  Z held       -> GRIPPER close"));
  Serial.println(F("  C+Z 0.7s    -> toggle demo"));
  Serial.println(F("  C+Z 1.8s    -> home + nunchuck mode"));
}

void printStatus()
{
  Serial.println();
  Serial.print(F("Mode: "));
  printModeName();
  if (controlMode == MODE_NUNCHUCK)
  {
    Serial.print(F(" | Layer: "));
    Serial.print(nunchuckLayer == 0 ? F("A (shoulder)") : F("B (elbow)"));
  }
  Serial.print(F(" | Sel: "));
  Serial.print((int)selectedJoint);
  Serial.print(F(" "));
  printJointName(selectedJoint);
  Serial.println();
  for (int i = 0; i < 6; i++)
  {
    Serial.print(F("  "));
    Serial.print(i);
    Serial.print(F(": "));
    printJointName(i);
    Serial.print(isContinuousJoint(i) ? F(" | cmd=") : F(" | cur="));
    Serial.print(currentAngle[i]);
    Serial.print(F(" | tgt="));
    Serial.println(targetAngle[i]);
  }
  Serial.print(F("  accel X="));
  Serial.print(accelX);
  Serial.print(F(" Y="));
  Serial.print(accelY);
  Serial.print(F(" Z="));
  Serial.println(accelZ);
  Serial.print(F("  "));
  printRecordingStatus();
  Serial.println();
}

void printJointName(uint8_t joint)
{
  switch (joint)
  {
  case BASE:
    Serial.print(F("base"));
    break;
  case SHOULDER:
    Serial.print(F("shoulder"));
    break;
  case ELBOW:
    Serial.print(F("elbow"));
    break;
  case WRIST_ROLL:
    Serial.print(F("wrist_roll"));
    break;
  case WRIST_PITCH:
    Serial.print(F("wrist_pitch"));
    break;
  case GRIPPER:
    Serial.print(F("gripper"));
    break;
  }
}

void printModeName()
{
  if (controlMode == MODE_SERIAL)
    Serial.print(F("serial"));
  else if (controlMode == MODE_DEMO)
    Serial.print(F("demo"));
  else if (controlMode == MODE_REPLAY)
    Serial.print(F("replay"));
  else
    Serial.print(F("nunchuck"));
}

void printCompactStatus()
{
  Serial.print(F("["));
  printModeName();
  if (controlMode == MODE_NUNCHUCK)
    Serial.print(nunchuckLayer == 0 ? F("/A") : F("/B"));
  Serial.print(F("] joint "));
  Serial.print((int)selectedJoint);
  Serial.print(F(" "));
  printJointName(selectedJoint);
  Serial.print(F(" | "));
  Serial.print(isContinuousJoint(selectedJoint) ? F("cmd=") : F("cur="));
  Serial.print(currentAngle[selectedJoint]);
  Serial.print(F(" | tgt="));
  Serial.println(targetAngle[selectedJoint]);
}

void printJointSummary(uint8_t joint)
{
  Serial.print(F("Joint "));
  Serial.print((int)joint);
  Serial.print(F(" "));
  printJointName(joint);
  Serial.print(isContinuousJoint(joint) ? F(" -> cmd=") : F(" -> cur="));
  Serial.print(currentAngle[joint]);
  Serial.print(F(", tgt="));
  Serial.println(targetAngle[joint]);
}

void printPinMap()
{
  Serial.println();
  Serial.println(F("Servo wiring:"));
  Serial.println(F("  0 base        -> D3  (continuous)"));
  Serial.println(F("  1 shoulder    -> D5"));
  Serial.println(F("  2 elbow       -> D6"));
  Serial.println(F("  3 wrist_roll  -> D9"));
  Serial.println(F("  4 wrist_pitch -> D10"));
  Serial.println(F("  5 gripper     -> D11"));
  Serial.println(F("External power: connect GND to Arduino GND."));
}

// ---------------------------------------------------------------------------
// Serial command parsing
// ---------------------------------------------------------------------------

bool tokenEquals(const char *a, const char *b)
{
  while (*a && *b)
  {
    char ca = (*a >= 'A' && *a <= 'Z') ? (*a - 'A' + 'a') : *a;
    char cb = (*b >= 'A' && *b <= 'Z') ? (*b - 'A' + 'a') : *b;
    if (ca != cb)
      return false;
    a++;
    b++;
  }
  return *a == '\0' && *b == '\0';
}

int parseJointToken(const char *token)
{
  if (token[0] >= '0' && token[0] <= '5' && token[1] == '\0')
    return token[0] - '0';
  if (tokenEquals(token, "base"))
    return BASE;
  if (tokenEquals(token, "shoulder"))
    return SHOULDER;
  if (tokenEquals(token, "elbow"))
    return ELBOW;
  if (tokenEquals(token, "wrist_roll"))
    return WRIST_ROLL;
  if (tokenEquals(token, "wrist_pitch"))
    return WRIST_PITCH;
  if (tokenEquals(token, "gripper"))
    return GRIPPER;
  return -1;
}

void jogSelectedJoint(int delta)
{
  setJointTarget(selectedJoint, targetAngle[selectedJoint] + delta);
}

void processSerialLine(char *line)
{
  char *argv[3] = {0};
  uint8_t argc = 0;
  char *tok = strtok(line, " ");
  while (tok && argc < 3)
  {
    argv[argc++] = tok;
    tok = strtok(NULL, " ");
  }
  if (argc == 0)
    return;

  // Serial mode exposes both help/status commands and joint control commands.
  // "set", "step", and shortcut jogs also make serial mode the active mode.
  if (tokenEquals(argv[0], "h") || tokenEquals(argv[0], "help"))
  {
    printHelp();
    return;
  }
  if (tokenEquals(argv[0], "p"))
  {
    printStatus();
    return;
  }
  if (tokenEquals(argv[0], "pins"))
  {
    printPinMap();
    return;
  }
  if (tokenEquals(argv[0], "selfcheck"))
  {
    runStartupSelfCheck();
    return;
  }

  if (tokenEquals(argv[0], "mode") && argc >= 2)
  {
    if (tokenEquals(argv[1], "s") || tokenEquals(argv[1], "serial"))
      controlMode = MODE_SERIAL;
    else if (tokenEquals(argv[1], "n") || tokenEquals(argv[1], "nunchuck"))
      controlMode = MODE_NUNCHUCK;
    else if (tokenEquals(argv[1], "d") || tokenEquals(argv[1], "demo"))
      controlMode = MODE_DEMO;
    else
    {
      Serial.println(F("unknown mode"));
      return;
    }
    printCompactStatus();
    return;
  }
  if (tokenEquals(argv[0], "rec") && argc >= 2)
  {
    if (tokenEquals(argv[1], "start"))
    {
      startRecording();
      return;
    }
    if (tokenEquals(argv[1], "stop"))
    {
      stopRecording();
      return;
    }
    if (tokenEquals(argv[1], "clear"))
    {
      clearRecording();
      printRecordingStatus();
      return;
    }
    if (tokenEquals(argv[1], "status"))
    {
      printRecordingStatus();
      return;
    }
    Serial.println(F("bad rec command"));
    return;
  }
  if (tokenEquals(argv[0], "play"))
  {
    startReplay();
    return;
  }
  if (tokenEquals(argv[0], "stop"))
  {
    if (controlMode == MODE_REPLAY)
    {
      controlMode = replayReturnMode;
      Serial.println(F("[replay] stopped"));
    }
    if (isRecording)
      stopRecording();
    return;
  }
  if (tokenEquals(argv[0], "sel") && argc >= 2)
  {
    int j = parseJointToken(argv[1]);
    if (j < 0)
    {
      Serial.println(F("bad joint"));
      return;
    }
    selectedJoint = (uint8_t)j;
    printCompactStatus();
    return;
  }
  if (tokenEquals(argv[0], "set") && argc >= 3)
  {
    int j = parseJointToken(argv[1]);
    if (j < 0)
    {
      Serial.println(F("bad joint"));
      return;
    }
    controlMode = MODE_SERIAL;
    setJointTarget((uint8_t)j, atoi(argv[2]));
    printJointSummary((uint8_t)j);
    return;
  }
  if (tokenEquals(argv[0], "step") && argc >= 3)
  {
    int j = parseJointToken(argv[1]);
    if (j < 0)
    {
      Serial.println(F("bad joint"));
      return;
    }
    controlMode = MODE_SERIAL;
    setJointTarget((uint8_t)j, targetAngle[j] + atoi(argv[2]));
    printJointSummary((uint8_t)j);
    return;
  }

  // Single-key shortcuts
  if (argc == 1 && argv[0][1] == '\0')
  {
    char key = argv[0][0];
    if (key >= '1' && key <= '6')
    {
      selectedJoint = (uint8_t)(key - '1');
      controlMode = MODE_SERIAL;
      printCompactStatus();
      return;
    }
    if (key == 'a')
    {
      controlMode = MODE_SERIAL;
      jogSelectedJoint(-2);
      printJointSummary(selectedJoint);
      return;
    }
    if (key == 'z')
    {
      controlMode = MODE_SERIAL;
      jogSelectedJoint(2);
      printJointSummary(selectedJoint);
      return;
    }
    if (key == 'm')
    {
      controlMode = MODE_SERIAL;
      printCompactStatus();
      return;
    }
    if (key == 'n')
    {
      controlMode = MODE_NUNCHUCK;
      printCompactStatus();
      return;
    }
    if (key == 'd')
    {
      controlMode = MODE_DEMO;
      printCompactStatus();
      return;
    }
  }

  Serial.println(F("unknown command"));
}

void handleSerialInput()
{
  while (Serial.available() > 0)
  {
    char c = (char)Serial.read();
    if (c == '\r')
      continue;
    if (c == '\n')
    {
      serialBuffer[serialLen] = '\0';
      processSerialLine(serialBuffer);
      serialLen = 0;
      continue;
    }
    if (serialLen + 1 < sizeof(serialBuffer) && c >= 32 && c <= 126)
      serialBuffer[serialLen++] = c;
    else if (serialLen + 1 >= sizeof(serialBuffer))
    {
      Serial.println(F("Input too long, line truncated."));
      serialLen = 0;
      // Defensive guard: drop the remaining bytes so the next command starts
      // from a clean line boundary.
      while (Serial.available() > 0 && Serial.read() != '\n') {}
    }
  }
}

// ---------------------------------------------------------------------------
// Control modes
// ---------------------------------------------------------------------------

// Detect a C-alone short tap (no Z, < C_TAP_MAX_MS) → switch layer A/B.
// cPressMs is also read by manualControl() to block the gripper during a tap.
void handleCTap()
{
  if (btnC && !cPrev)
  {
    cPressMs = millis();
    cTapValid = !btnZ; // invalid if Z was already held when C went down
  }
  if (cTapValid && btnZ)
  {
    cTapValid = false; // Z came in during the hold → not a solo tap
  }
  if (!btnC && cPrev && cTapValid)
  {
    if (millis() - cPressMs < C_TAP_MAX_MS)
    {
      nunchuckLayer ^= 1;
      Serial.print(F("[layer] "));
      Serial.println(nunchuckLayer == 0
                         ? F("A: joy-Y=shoulder | tilt=wrist")
                         : F("B: joy-Y=elbow   | tilt=wrist"));
    }
    cTapValid = false;
  }
  cPrev = btnC;
}

void manualControl()
{
  // Manual control combines two input sources:
  // - joystick X/Y for base and shoulder/elbow
  // - accelerometer tilt for wrist roll/pitch
  // The function computes targets only; `smoothMoveStep()` applies the actual
  // rate-limited motion afterward.
  //
  // These constants are empirical tuning values for the current hardware, not
  // protocol-level values from the Nunchuck itself.
  const int deadzone = 8;
  const int maxBaseDelta = 5;
  const int maxJointDeltaPerTick = 3;
  const int accelMin = 300;
  const int accelMax = 700;

  static unsigned long lastManualControlMs = 0;
  static float gripperAccumulator = 0.0f;
  static float filteredAccelX = 512.0f;
  static float filteredAccelY = 512.0f;
  unsigned long now = millis();
  unsigned long elapsedMs = (lastManualControlMs == 0) ? CONTROL_INTERVAL_MS : (now - lastManualControlMs);
  lastManualControlMs = now;

  int deltaX = mapJoystickToDelta(joyX, deadzone, maxBaseDelta);
  int deltaY = mapJoystickToDelta(joyY, deadzone, maxJointDeltaPerTick);

  // BASE: always joystick X
  targetAngle[BASE] = BASE_STOP_COMMAND + (deltaX * 4);
  targetAngle[BASE] = constrain(targetAngle[BASE], BASE_MIN_COMMAND, BASE_MAX_COMMAND);

  // Layer A → SHOULDER | Layer B → ELBOW
  if (nunchuckLayer == 0)
  {
    targetAngle[SHOULDER] = currentAngle[SHOULDER] + deltaY;
  }
  else
  {
    targetAngle[ELBOW] = currentAngle[ELBOW] + deltaY;
  }

  // WRIST_ROLL ← tilt X, WRIST_PITCH ← tilt Y
  int clampedAccelX = constrain(accelX, accelMin, accelMax);
  int clampedAccelY = constrain(accelY, accelMin, accelMax);
  // Filter accelerometer motion before mapping it to wrist targets so small
  // sensor noise does not immediately become servo jitter.
  filteredAccelX += (clampedAccelX - filteredAccelX) * WRIST_FILTER_ALPHA;
  filteredAccelY += (clampedAccelY - filteredAccelY) * WRIST_FILTER_ALPHA;
  int wristRollTarget = map((int)filteredAccelX, accelMin, accelMax, 0, 180);
  // Keep wrist pitch aligned with the controller: tilt forward = wrist up.
  int wristPitchTarget = map((int)filteredAccelY, accelMin, accelMax, 20, 160);
  if (abs(wristRollTarget - targetAngle[WRIST_ROLL]) >= WRIST_TARGET_DEADBAND)
    targetAngle[WRIST_ROLL] = wristRollTarget;
  if (abs(wristPitchTarget - targetAngle[WRIST_PITCH]) >= WRIST_TARGET_DEADBAND)
    targetAngle[WRIST_PITCH] = wristPitchTarget;

  // GRIPPER: C held long enough (> C_TAP_MAX_MS) → open; Z alone → close.
  // The delay prevents a layer-switch tap from accidentally moving the gripper.
  float gripperDelta = 0.0f;
  bool cHeld = btnC && !btnZ && (millis() - cPressMs >= C_TAP_MAX_MS);
  if (cHeld)
    gripperDelta = (elapsedMs / 1000.0f) * GRIPPER_SPEED_PER_SEC;
  else if (btnZ && !btnC)
    gripperDelta = -(elapsedMs / 1000.0f) * GRIPPER_SPEED_PER_SEC;
  else
    gripperAccumulator = 0.0f;

  gripperAccumulator += gripperDelta;
  if (gripperAccumulator >= 1.0f)
  {
    int step = (int)gripperAccumulator;
    targetAngle[GRIPPER] += step;
    gripperAccumulator -= step;
  }
  else if (gripperAccumulator <= -1.0f)
  {
    int step = (int)(-gripperAccumulator);
    targetAngle[GRIPPER] -= step;
    gripperAccumulator += step;
  }

  applyJointLimits();
}

void demoControl()
{
  unsigned long t = millis();
  targetAngle[BASE] = BASE_STOP_COMMAND + (int)(12.0 * sin(t / 900.0));
  targetAngle[SHOULDER] = 90 + (int)(20.0 * sin(t / 700.0));
  targetAngle[ELBOW] = 90 + (int)(15.0 * sin(t / 600.0));
  targetAngle[WRIST_ROLL] = 90 + (int)(35.0 * sin(t / 1000.0));
  targetAngle[WRIST_PITCH] = 90 + (int)(20.0 * sin(t / 1200.0));
  targetAngle[GRIPPER] = 70 + (int)(15.0 * sin(t / 500.0));
  applyJointLimits();
}

void handleButtonGestures()
{
  // Combo gestures are resolved here before the motion step so they can switch
  // mode or reset the arm without waiting for the next manual control update.
  bool comboNow = btnC && btnZ;

  if (comboNow && !comboPrev)
  {
    comboStartMs = millis();
    comboActionDone = false;
  }
  if (comboNow && !comboActionDone)
  {
    unsigned long held = millis() - comboStartMs;
    if (held >= HOME_HOLD_MS)
    {
      setHomePosition();
      controlMode = MODE_NUNCHUCK;
      nunchuckLayer = 0;
      comboActionDone = true;
    }
    else if (held >= DEMO_TOGGLE_HOLD_MS)
    {
      controlMode = (controlMode == MODE_DEMO) ? MODE_NUNCHUCK : MODE_DEMO;
      comboActionDone = true;
    }
  }
  if (!comboNow && comboPrev)
    comboActionDone = false;
  comboPrev = comboNow;
}

void setup()
{
  Serial.begin(115200);
  initNunchuck();

  for (int i = 0; i < 6; i++)
  {
    currentAngle[i] = isContinuousJoint(i) ? BASE_STOP_COMMAND : homeAngle[i];
    targetAngle[i] = currentAngle[i];
    servos[i].attach(SERVO_PINS[i]);
    servos[i].write(toServoCommand(i, currentAngle[i]));
  }

  runStartupSelfCheck();
  lastControlMs = millis();
  Serial.println(F("Robotic arm controller started."));
  printHelp();
  printCompactStatus();
}

void loop()
{
  // Control flow is: read input, resolve gestures/mode, compute targets, then
  // execute one smooth motion step.
  handleSerialInput();

  if (millis() - lastControlMs < CONTROL_INTERVAL_MS)
    return;
  lastControlMs = millis();

  if (controlMode != MODE_SERIAL && controlMode != MODE_REPLAY)
  {
    bool ok = readNunchuck();
    if (!ok)
    {
      smoothMoveStep();
      return;
    }
    handleButtonGestures();
    handleCTap();
  }

  if (controlMode == MODE_DEMO)
    demoControl();
  else if (controlMode == MODE_SERIAL)
  { /* targets driven by serial */
  }
  else if (controlMode == MODE_REPLAY)
    replayControl();
  else
    manualControl();

  captureRecordFrame(false);
  smoothMoveStep();
}
