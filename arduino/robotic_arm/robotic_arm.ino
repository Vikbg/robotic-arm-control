#include <Servo.h>
#include <Wire.h>

// Set to 1 to use WiiChuck.h adapter path (if installed and API compatible).
// Set to 0 to use the built-in raw I2C Nunchuck reader.
#define USE_WIICHUCK_LIB 0

#if USE_WIICHUCK_LIB
#include <WiiChuck.h>
WiiChuck nunchuck;
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

int currentAngle[6] = {90, 90, 90, 90, 90, 60};
int targetAngle[6] = {90, 90, 90, 90, 90, 60};

const int minAngle[6] = {10, 20, 20, 0, 20, 20};
const int maxAngle[6] = {170, 160, 160, 180, 160, 110};
const int homeAngle[6] = {90, 90, 90, 90, 90, 60};

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

void nunchuckWrite(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(0x52);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void initNunchuck() {
#if USE_WIICHUCK_LIB
  nunchuck.begin();
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
  // Note: API names can vary depending on WiiChuck library version.
  // If this block fails to compile, keep USE_WIICHUCK_LIB at 0 or adapt names.
  if (!nunchuck.update()) {
    return false;
  }

  joyX = nunchuck.readJoyX();
  joyY = nunchuck.readJoyY();
  btnC = nunchuck.buttonC();
  btnZ = nunchuck.buttonZ();
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
    if (targetAngle[i] < minAngle[i]) {
      targetAngle[i] = minAngle[i];
    }
    if (targetAngle[i] > maxAngle[i]) {
      targetAngle[i] = maxAngle[i];
    }
  }
}

void smoothMoveStep() {
  for (int i = 0; i < 6; i++) {
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
    targetAngle[i] = homeAngle[i];
  }
}

void manualControl() {
  const int deadzone = 8;
  const int maxDeltaPerTick = 3;

  int deltaX = mapJoystickToDelta(joyX, deadzone, maxDeltaPerTick);
  int deltaY = mapJoystickToDelta(joyY, deadzone, maxDeltaPerTick);

  targetAngle[BASE] += deltaX;
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

  targetAngle[BASE] = 90 + (int)(30.0 * sin(t / 900.0));
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

  Serial.println("Robotic arm controller started.");
}

void loop() {
  if (millis() - lastControlMs < CONTROL_INTERVAL_MS) {
    return;
  }
  lastControlMs = millis();

  bool ok = readNunchuck();

  if (!ok) {
    // Fail-safe: keep current targets; do not apply abrupt changes.
    smoothMoveStep();
    return;
  }

  handleButtonGestures();

  if (demoMode) {
    demoControl();
  } else {
    manualControl();
  }

  smoothMoveStep();
}
