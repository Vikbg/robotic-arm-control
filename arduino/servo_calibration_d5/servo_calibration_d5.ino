// Single-servo calibration helper for the D5 output.
// Use this sketch to probe a continuous servo's stop point or a positional
// servo's angle range without involving the full robotic arm controller.
#include <Servo.h>

Servo testServo;

// D5 is fixed here so the sketch always drives the same output pin while you
// tune one servo on the breadboard.
const uint8_t SERVO_PIN = 5;
// 90 is the neutral/stop reference for continuous servos and a midpoint test
// value for positional servos.
int servoCommand = 90;

void printHelp() {
  Serial.println(F("D5 servo calibration"));
  Serial.println(F("Commands:"));
  Serial.println(F("  h           show help"));
  Serial.println(F("  p           print current command"));
  Serial.println(F("  0           stop/center at 90"));
  Serial.println(F("  a           command -1"));
  Serial.println(F("  z           command +1"));
  Serial.println(F("  q           command -5"));
  Serial.println(F("  s           command +5"));
  Serial.println(F("  set <value> set exact command"));
  Serial.println(F("Notes:"));
  Serial.println(F("  Continuous servo: 90 is usually stop, <90 one way, >90 the other."));
  Serial.println(F("  Positional servo: values are angles."));
}

void printStatus() {
  Serial.print(F("D5 command = "));
  Serial.println(servoCommand);
}

void applyCommand() {
  if (servoCommand < 0) {
    servoCommand = 0;
  }
  if (servoCommand > 180) {
    servoCommand = 180;
  }

  testServo.write(servoCommand);
  printStatus();
}

// Newline-terminated commands are used for exact values and help/status.
void processLine(char *line) {
  char *cmd = strtok(line, " ");
  char *arg = strtok(NULL, " ");

  if (cmd == NULL) {
    return;
  }

  if (strcmp(cmd, "h") == 0 || strcmp(cmd, "help") == 0) {
    printHelp();
    return;
  }

  if (strcmp(cmd, "p") == 0) {
    printStatus();
    return;
  }

  if (strcmp(cmd, "set") == 0 && arg != NULL) {
    servoCommand = atoi(arg);
    applyCommand();
    return;
  }

  Serial.println(F("Unknown command"));
}

// Single-key shortcuts are applied immediately; longer text commands are
// accumulated in a fixed buffer so the sketch stays tiny on AVR RAM.
void handleSerial() {
  static char buffer[24];
  static uint8_t len = 0;

  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      buffer[len] = '\0';
      processLine(buffer);
      len = 0;
      continue;
    }

    if (c == '0') {
      servoCommand = 90;
      applyCommand();
      continue;
    }

    if (c == 'a') {
      servoCommand -= 1;
      applyCommand();
      continue;
    }

    if (c == 'z') {
      servoCommand += 1;
      applyCommand();
      continue;
    }

    if (c == 'q') {
      servoCommand -= 5;
      applyCommand();
      continue;
    }

    if (c == 's') {
      servoCommand += 5;
      applyCommand();
      continue;
    }

    if (c == 'p') {
      printStatus();
      continue;
    }

    if (c == 'h' && len == 0) {
      printHelp();
      continue;
    }

    if (c >= 32 && c <= 126 && len + 1 < sizeof(buffer)) {
      buffer[len++] = c;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // Start from 90 so the servo does not jump unexpectedly at boot.
  testServo.attach(SERVO_PIN);
  testServo.write(90);

  Serial.println(F("Servo test on D5 started."));
  printHelp();
  printStatus();
}

void loop() {
  handleSerial();
}
