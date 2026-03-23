#include <Servo.h>
#include <Wire.h>

// Set to 1 to use WiiChuck.h adapter path (if installed and API compatible).
// Set to 0 to use the built-in raw I2C Nunchuck reader.
#define USE_WIICHUCK_LIB 0

#if USE_WIICHUCK_LIB
#include <WiiChuck.h>
Accessory nunchuck;
#endif

// Servo pins (UNO PWM-capable pins)
const uint8_t SERVO_PINS[6] = {3, 5, 6, 9, 10, 11};

enum Joint {
  BASE = 0,
  SHOULDER,
  ELBOW,
  WRIST_ROLL,
  WRIST_PITCH,
  GRIPPER
};

Servo servos[6];

// For the base (continuous rotation), these arrays store the speed command.
int currentAngle[6] = {90, 140, 90, 90, 90, 60};
int targetAngle[6] = {90, 140, 90, 90, 90, 60};

const int minAngle[6] = {0, 20, 20, -20, 20, 20};
const int maxAngle[6] = {180, 160, 160, 180, 160, 75};
const int homeAngle[6] = {90, 140, 90, 90, 90, 60};
const int BASE_STOP_COMMAND = 90;
const int BASE_MIN_COMMAND = 70;
const int BASE_MAX_COMMAND = 110;

// Smoothing: max degrees per control step
const int SMOOTH_STEP = 2;
const unsigned long CONTROL_INTERVAL_MS = 20;
unsigned long lastControlMs = 0;

// Nunchuck state
int joyX = 128;
int joyY = 128;
bool btnC = false;
bool btnZ = false;

// Button gesture handling
bool comboPrev = false;
unsigned long comboStartMs = 0;
const unsigned long DEMO_TOGGLE_HOLD_MS = 700;
const unsigned long HOME_HOLD_MS = 1800;
bool demoMode = false;
bool comboActionDone = false;

enum ControlMode {
  MODE_NUNCHUCK = 0,
  MODE_SERIAL,
  MODE_DEMO
};

ControlMode controlMode = MODE_NUNCHUCK;
uint8_t selectedJoint = BASE;

char serialBuffer[32];
uint8_t serialLen = 0;

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

void nunchuckWrite(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(0x52);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void initNunchuck() {
#if USE_WIICHUCK_LIB
  nunchuck.begin();
  if (nunchuck.type == UnknownChuck) {
    nunchuck.type = NUNCHUCK;
  }
  delay(50);
#else
  Wire.begin();
  delay(50);

  // Standard init sequence for most Nunchuck-compatible modules.
  nunchuckWrite(0xF0, 0x55);
  delay(1);
  nunchuckWrite(0xFB, 0x00);
  delay(1);
#endif
}

bool readNunchuck() {
#if USE_WIICHUCK_LIB
  if (!nunchuck.readData()) {
    return false;
  }

  joyX = nunchuck.getJoyX();
  joyY = nunchuck.getJoyY();
  btnC = nunchuck.getButtonC();
  btnZ = nunchuck.getButtonZ();
  return true;
#else
  uint8_t data[6] = {0};

  Wire.beginTransmission(0x52);
  Wire.write((uint8_t)0x00);
  if (Wire.endTransmission() != 0) {
    return false;
  }

  delayMicroseconds(500);

  int bytes = Wire.requestFrom(0x52, 6);
  if (bytes != 6) {
    return false;
  }

  for (int i = 0; i < 6; i++) {
    data[i] = Wire.read();
  }

  joyX = data[0];
  joyY = data[1];

  // Active-low button bits in byte 5
  btnZ = !(data[5] & 0x01);
  btnC = !((data[5] >> 1) & 0x01);

  return true;
#endif
}

int mapJoystickToDelta(int value, int deadzone, int maxDelta) {
  int centered = value - 128;
  if (abs(centered) < deadzone) {
    return 0;
  }

  long delta = map(centered, -128, 127, -maxDelta, maxDelta);
  return (int)delta;
}

void applyJointLimits() {
  for (int i = 0; i < 6; i++) {
    if (isContinuousJoint(i)) {
      if (targetAngle[i] < BASE_MIN_COMMAND) {
        targetAngle[i] = BASE_MIN_COMMAND;
      }
      if (targetAngle[i] > BASE_MAX_COMMAND) {
        targetAngle[i] = BASE_MAX_COMMAND;
      }
    } else {
      if (targetAngle[i] < minAngle[i]) {
        targetAngle[i] = minAngle[i];
      }
      if (targetAngle[i] > maxAngle[i]) {
        targetAngle[i] = maxAngle[i];
      }
    }
  }
}

void setJointTarget(uint8_t joint, int angle) {
  if (joint >= 6) {
    return;
  }
  targetAngle[joint] = angle;
  applyJointLimits();
}

bool isContinuousJoint(uint8_t joint) {
  return joint == BASE;
}

void smoothMoveStep() {
  for (int i = 0; i < 6; i++) {
    if (isContinuousJoint(i)) {
      currentAngle[i] = targetAngle[i];
      servos[i].write(currentAngle[i]);
      continue;
    }

    int diff = targetAngle[i] - currentAngle[i];

    if (diff > SMOOTH_STEP) {
      currentAngle[i] += SMOOTH_STEP;
    } else if (diff < -SMOOTH_STEP) {
      currentAngle[i] -= SMOOTH_STEP;
    } else {
      currentAngle[i] = targetAngle[i];
    }

    servos[i].write(currentAngle[i]);
  }
}

void setHomePosition() {
  for (int i = 0; i < 6; i++) {
    if (isContinuousJoint(i)) {
      targetAngle[i] = BASE_STOP_COMMAND;
    } else {
      targetAngle[i] = homeAngle[i];
    }
  }
}

void printHelp() {
  Serial.println(F("Commands:"));
  Serial.println(F("  h/help       show help"));
  Serial.println(F("  p            print status"));
  Serial.println(F("  pins         show servo wiring"));
  Serial.println(F("  mode s|n|d   serial/nunchuck/demo"));
  Serial.println(F("  sel <0-5>    select joint"));
  Serial.println(F("  set <j> <a>  set joint angle/speed cmd"));
  Serial.println(F("  step <j> <d> add delta to joint"));
  Serial.println(F("Keys:"));
  Serial.println(F("  1-6 select joint, a/z jog -/+ , m serial, n nunchuck, d demo"));
  Serial.println(F("Base note: joint 0 is continuous servo, 90=stop, <90 one way, >90 other way."));
}

void printStatus() {
  Serial.println();
  Serial.print(F("Mode: "));
  printModeName();
  Serial.print(F(" | Selected: "));
  Serial.print((int)selectedJoint);
  Serial.print(F(" "));
  printJointName(selectedJoint);
  Serial.println();

  for (int i = 0; i < 6; i++) {
    Serial.print(F("  "));
    Serial.print(i);
    Serial.print(F(": "));
    printJointName(i);
    if (isContinuousJoint(i)) {
      Serial.print(F(" | cmd="));
    } else {
      Serial.print(F(" | cur="));
    }
    Serial.print(currentAngle[i]);
    Serial.print(F(" | tgt="));
    Serial.println(targetAngle[i]);
  }
  Serial.println();
  Serial.println(F("Press 1-6 to select, a/z to move, p for full status."));
}

void printJointName(uint8_t joint) {
  switch (joint) {
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

void printModeName() {
  if (controlMode == MODE_SERIAL) {
    Serial.print(F("serial"));
  } else if (controlMode == MODE_DEMO) {
    Serial.print(F("demo"));
  } else {
    Serial.print(F("nunchuck"));
  }
}

void printCompactStatus() {
  Serial.print(F("["));
  printModeName();
  Serial.print(F("] joint "));
  Serial.print((int)selectedJoint);
  Serial.print(F(" "));
  printJointName(selectedJoint);
  Serial.print(F(" | "));
  if (isContinuousJoint(selectedJoint)) {
    Serial.print(F("cmd="));
  } else {
    Serial.print(F("cur="));
  }
  Serial.print(currentAngle[selectedJoint]);
  Serial.print(F(" | tgt="));
  Serial.println(targetAngle[selectedJoint]);
}

void printJointSummary(uint8_t joint) {
  Serial.print(F("Joint "));
  Serial.print((int)joint);
  Serial.print(F(" "));
  printJointName(joint);
  if (isContinuousJoint(joint)) {
    Serial.print(F(" -> cmd="));
  } else {
    Serial.print(F(" -> cur="));
  }
  Serial.print(currentAngle[joint]);
  Serial.print(F(", tgt="));
  Serial.println(targetAngle[joint]);
}

void printPinMap() {
  Serial.println();
  Serial.println(F("Servo wiring:"));
  Serial.println(F("  0 base        -> D3"));
  Serial.println(F("  1 shoulder    -> D5"));
  Serial.println(F("  2 elbow       -> D6"));
  Serial.println(F("  3 wrist_roll  -> D9"));
  Serial.println(F("  4 wrist_pitch -> D10"));
  Serial.println(F("  5 gripper     -> D11"));
  Serial.println(F("Power: servo red -> 5V/external +, brown/black -> GND, signal -> Dx"));
  Serial.println(F("Important: if using external servo power, connect external GND to Arduino GND."));
  Serial.println(F("Base servo: continuous rotation on D3, use command near 90 to stop."));
}

bool tokenEquals(const char *a, const char *b) {
  while (*a && *b) {
    char ca = *a;
    char cb = *b;
    if (ca >= 'A' && ca <= 'Z') {
      ca = ca - 'A' + 'a';
    }
    if (cb >= 'A' && cb <= 'Z') {
      cb = cb - 'A' + 'a';
    }
    if (ca != cb) {
      return false;
    }
    a++;
    b++;
  }
  return *a == '\0' && *b == '\0';
}

int parseJointToken(const char *token) {
  if (token[0] >= '0' && token[0] <= '5' && token[1] == '\0') {
    return token[0] - '0';
  }

  for (int i = 0; i < 6; i++) {
    if ((i == BASE && tokenEquals(token, "base")) ||
        (i == SHOULDER && tokenEquals(token, "shoulder")) ||
        (i == ELBOW && tokenEquals(token, "elbow")) ||
        (i == WRIST_ROLL && tokenEquals(token, "wrist_roll")) ||
        (i == WRIST_PITCH && tokenEquals(token, "wrist_pitch")) ||
        (i == GRIPPER && tokenEquals(token, "gripper"))) {
      return i;
    }
  }

  return -1;
}

void jogSelectedJoint(int delta) {
  setJointTarget(selectedJoint, targetAngle[selectedJoint] + delta);
}

void processSerialLine(char *line) {
  char *argv[3] = {0};
  uint8_t argc = 0;
  char *token = strtok(line, " ");

  while (token != NULL && argc < 3) {
    argv[argc++] = token;
    token = strtok(NULL, " ");
  }

  if (argc == 0) {
    return;
  }

  if (tokenEquals(argv[0], "h") || tokenEquals(argv[0], "help")) {
    printHelp();
    return;
  }

  if (tokenEquals(argv[0], "p")) {
    printStatus();
    return;
  }

  if (tokenEquals(argv[0], "pins")) {
    printPinMap();
    return;
  }

  if (tokenEquals(argv[0], "mode") && argc >= 2) {
    if (tokenEquals(argv[1], "s") || tokenEquals(argv[1], "serial")) {
      controlMode = MODE_SERIAL;
    } else if (tokenEquals(argv[1], "n") || tokenEquals(argv[1], "nunchuck")) {
      controlMode = MODE_NUNCHUCK;
    } else if (tokenEquals(argv[1], "d") || tokenEquals(argv[1], "demo")) {
      controlMode = MODE_DEMO;
    } else {
      Serial.println(F("unknown mode"));
      return;
    }
    printCompactStatus();
    return;
  }

  if (tokenEquals(argv[0], "sel") && argc >= 2) {
    int joint = parseJointToken(argv[1]);
    if (joint < 0) {
      Serial.println(F("bad joint"));
      return;
    }
    selectedJoint = (uint8_t)joint;
    printCompactStatus();
    return;
  }

  if (tokenEquals(argv[0], "set") && argc >= 3) {
    int joint = parseJointToken(argv[1]);
    int angle = atoi(argv[2]);
    if (joint < 0) {
      Serial.println(F("bad joint"));
      return;
    }
    controlMode = MODE_SERIAL;
    setJointTarget((uint8_t)joint, angle);
    printJointSummary((uint8_t)joint);
    return;
  }

  if (tokenEquals(argv[0], "step") && argc >= 3) {
    int joint = parseJointToken(argv[1]);
    int delta = atoi(argv[2]);
    if (joint < 0) {
      Serial.println(F("bad joint"));
      return;
    }
    controlMode = MODE_SERIAL;
    setJointTarget((uint8_t)joint, targetAngle[joint] + delta);
    printJointSummary((uint8_t)joint);
    return;
  }

  Serial.println(F("unknown command"));
}

void handleSerialInput() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      serialBuffer[serialLen] = '\0';
      processSerialLine(serialBuffer);
      serialLen = 0;
      continue;
    }

    if (serialLen == 0) {
      int next = Serial.peek();
      bool singleKey = (next == -1 || next == '\r' || next == '\n');
      if (!singleKey) {
        // Fall through to buffer the character for a multi-char command.
      } else if (c >= '1' && c <= '6') {
        selectedJoint = (uint8_t)(c - '1');
        controlMode = MODE_SERIAL;
        printCompactStatus();
        continue;
      } else if (c == 'a') {
        controlMode = MODE_SERIAL;
        jogSelectedJoint(-2);
        printJointSummary(selectedJoint);
        continue;
      } else if (c == 'z') {
        controlMode = MODE_SERIAL;
        jogSelectedJoint(2);
        printJointSummary(selectedJoint);
        continue;
      } else if (c == 'm') {
        controlMode = MODE_SERIAL;
        printCompactStatus();
        continue;
      } else if (c == 'n') {
        controlMode = MODE_NUNCHUCK;
        printCompactStatus();
        continue;
      } else if (c == 'd') {
        controlMode = MODE_DEMO;
        printCompactStatus();
        continue;
      } else if (c == 'p') {
        printStatus();
        continue;
      } else if (c == 'h') {
        printHelp();
        continue;
      }

    }

    if (serialLen + 1 < sizeof(serialBuffer) && c >= 32 && c <= 126) {
      serialBuffer[serialLen++] = c;
    }
  }
}

void manualControl() {
  const int deadzone = 8;
  const int maxDeltaPerTick = 3;

  int deltaX = mapJoystickToDelta(joyX, deadzone, maxDeltaPerTick);
  int deltaY = mapJoystickToDelta(joyY, deadzone, maxDeltaPerTick);

  targetAngle[BASE] = BASE_STOP_COMMAND + (deltaX * 4);
  targetAngle[SHOULDER] += deltaY;

  if (btnC && !btnZ) {
    targetAngle[GRIPPER] += 2;
  } else if (btnZ && !btnC) {
    targetAngle[GRIPPER] -= 2;
  }

  applyJointLimits();
}

void demoControl() {
  static unsigned long startMs = millis();
  unsigned long t = millis() - startMs;

  targetAngle[BASE] = BASE_STOP_COMMAND + (int)(12.0 * sin(t / 900.0));
  targetAngle[SHOULDER] = 90 + (int)(20.0 * sin(t / 700.0));
  targetAngle[ELBOW] = 90 + (int)(15.0 * sin(t / 600.0));
  targetAngle[WRIST_ROLL] = 90 + (int)(35.0 * sin(t / 1000.0));
  targetAngle[WRIST_PITCH] = 90 + (int)(20.0 * sin(t / 1200.0));
  targetAngle[GRIPPER] = 70 + (int)(15.0 * sin(t / 500.0));

  applyJointLimits();
}

void handleButtonGestures() {
  bool comboNow = btnC && btnZ;

  if (comboNow && !comboPrev) {
    comboStartMs = millis();
    comboActionDone = false;
  }

  if (comboNow && !comboActionDone) {
    unsigned long held = millis() - comboStartMs;

    if (held >= HOME_HOLD_MS) {
      setHomePosition();
      targetAngle[BASE] = BASE_STOP_COMMAND;
      comboActionDone = true;
    } else if (held >= DEMO_TOGGLE_HOLD_MS) {
      demoMode = !demoMode;
      comboActionDone = true;
    }
  }

  if (!comboNow && comboPrev) {
    comboActionDone = false;
  }

  comboPrev = comboNow;
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 6; i++) {
    servos[i].attach(SERVO_PINS[i]);
    servos[i].write(currentAngle[i]);
  }

  initNunchuck();
  setHomePosition();
  currentAngle[BASE] = BASE_STOP_COMMAND;
  targetAngle[BASE] = BASE_STOP_COMMAND;

  Serial.println(F("Robotic arm controller started."));
  printHelp();
  printCompactStatus();
}

void loop() {
  handleSerialInput();

  if (millis() - lastControlMs < CONTROL_INTERVAL_MS) {
    return;
  }
  lastControlMs = millis();

  if (controlMode == MODE_DEMO) {
    demoControl();
  } else if (controlMode == MODE_SERIAL) {
    // Targets are updated by serial commands and keyboard jog input.
  } else {
    bool ok = readNunchuck();
    if (!ok) {
      smoothMoveStep();
      return;
    }

    handleButtonGestures();
    if (demoMode) {
      controlMode = MODE_DEMO;
      demoControl();
    } else {
      controlMode = MODE_NUNCHUCK;
      manualControl();
    }
  }

  smoothMoveStep();
}
