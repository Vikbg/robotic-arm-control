# 6-DOF Robotic Arm Control System

## Overview

Arduino Uno project for controlling a 6-DOF robotic arm with:
- 3x MG996R (high torque): base, shoulder, elbow
- 3x SG90 (micro): wrist roll, wrist pitch, gripper
- 1x Wii Nunchuck (joystick + 2 buttons) as the primary controller

Control is real-time in manual mode, with:
- Home reset
- Demo/auto motion mode
- Angle limits and smoothing for safer motion

## System Architecture

Wii Nunchuck (I2C)
-> Arduino Uno
-> PWM outputs (D3 D5 D6 D9 D10 D11)
-> 6x Servo motors

Power:
- External 5V-6V power supply for servos
- USB for Arduino logic
- Shared GND between Arduino and servo power

Important: do not power 6 servos from Arduino 5V pin.

## Pin Mapping

### Servos
- D3: base
- D5: shoulder
- D6: elbow
- D9: wrist roll
- D10: wrist pitch
- D11: gripper

### Nunchuck (I2C)
- SDA: A4
- SCL: A5
- VCC: 3.3V or 5V (depends on your module)
- GND: GND

## Controls

- Joystick X: base continuous-rotation speed
- Joystick Y: shoulder on layer A, elbow on layer B
- Tilt X: wrist roll
- Tilt Y: wrist pitch
- Tap C: switch layer A/B
- Hold C: open gripper
- Hold Z: close gripper
- Hold C + Z for about 0.7s: toggle demo mode
- Hold C + Z for about 1.8s: home position and return to nunchuck mode

## Build And Upload

From project root:

```bash
arduino-cli compile --fqbn arduino:avr:uno arduino/robotic_arm
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno arduino/robotic_arm
```

If you keep a local copy in `./bin/arduino-cli`, replace `arduino-cli` with that path.

## WiiChuck.h Mode (Optional)

Default mode is raw I2C reader (no extra library required).

If you want to use `WiiChuck.h`:
1. Install library:
```bash
arduino-cli lib install WiiChuck
```
2. In [robotic_arm.ino](/home/viktor_srhk/Documents/HighSchool/Personal/SI-CIT/robotic-arm-control/arduino/robotic_arm/robotic_arm.ino), set:
```cpp
#define USE_WIICHUCK_LIB 1
```
3. Compile.

If your WiiChuck library version uses different method names, keep `USE_WIICHUCK_LIB` at `0` or adapt the 4 calls in `readNunchuck()`.

## Repository Structure

```text
robotic-arm-control/
├── arduino/
│   └── robotic_arm/
│       └── robotic_arm.ino
├── README.md
├── REQUIREMENTS_SPECIFICATION.md
└── TODO.md
```

## Next Steps

- Mechanical calibration per servo
- End-stop and collision-safe limits
- Serial CLI for diagnostics
- Optional Rust desktop/controller bridge
