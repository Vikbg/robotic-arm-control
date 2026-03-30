# 6-DOF Robotic Arm Control System

## Overview

This repository contains an Arduino Uno based controller for a 6-DOF robotic arm driven by a Wii Nunchuck.

The project currently includes:

- a main runtime sketch for the full arm
- a single-servo calibration sketch for quick bench testing
- wiring and simulation assets
- project requirements, TODO tracking, and detailed technical documentation

Main hardware assumptions:

- `3x MG996R` for base, shoulder, and elbow
- `3x SG90` for wrist roll, wrist pitch, and gripper
- `1x Wii Nunchuck` connected over I2C
- external `5V-6V` servo power supply with shared ground

Important: do not power all six servos from the Arduino `5V` pin.

## Features

- Live Nunchuck control with joystick, accelerometer tilt, and button gestures
- Three control modes: `NUNCHUCK`, `SERIAL`, and `DEMO`
- Per-joint angle limits and home positions
- Software smoothing, wrist filtering, and gripper speed control
- Serial diagnostics commands for testing and manual jogging
- A separate D5 servo calibration utility
- Wokwi simulation support using prebuilt firmware artifacts

## Quick Start

Install the Arduino AVR core if needed:

```bash
arduino-cli config init
arduino-cli core update-index
arduino-cli core install arduino:avr
```

Build and upload the main sketch from the repository root:

```bash
arduino-cli compile --fqbn arduino:avr:uno arduino/robotic_arm
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno arduino/robotic_arm
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

If you keep a local copy in `./bin/arduino-cli`, replace `arduino-cli` with that path.

## Control Summary

Nunchuck controls:

- Joystick X -> base continuous-rotation speed
- Joystick Y -> shoulder in layer A, elbow in layer B
- Tilt X -> wrist roll
- Tilt Y -> wrist pitch
- Tap `C` -> switch between layer A and layer B
- Hold `C` -> open gripper
- Hold `Z` -> close gripper
- Hold `C + Z` about `0.7s` -> toggle demo mode
- Hold `C + Z` about `1.8s` -> move to home position and return to Nunchuck mode

Implementation notes:

- The base is treated as a continuous servo around command `90`.
- Wrist pitch is software-reversed at write time to match the current physical mounting.
- Wrist motion is filtered to reduce accelerometer jitter.

## Serial Diagnostics

The main sketch exposes a serial command interface for testing and debugging.

Supported commands:

- `h` / `help`
- `p`
- `pins`
- `mode s|n|d`
- `sel <0-5>`
- `set <joint> <angle>`
- `step <joint> <delta>`

Single-key shortcuts:

- `1..6` select joint
- `a` / `z` jog selected joint by `-2` / `+2`
- `m` / `n` / `d` switch to serial / nunchuck / demo mode

## Repository Layout

```text
robotic-arm-control/
├── arduino/
│   ├── robotic_arm/
│   │   └── robotic_arm.ino
│   └── servo_calibration_d5/
│       └── servo_calibration_d5.ino
├── docs/
│   ├── calibration.md
│   ├── control-system.md
│   ├── development.md
│   ├── hardware.md
│   └── troubleshooting.md
├── README.md
├── NOTES.md
├── REQUIREMENTS_SPECIFICATION.md
├── TODO.md
├── diagram.json
└── wokwi.toml
```

## Documentation

Start here for deeper information:

- [Hardware Guide](docs/hardware.md)
- [Control System](docs/control-system.md)
- [Calibration](docs/calibration.md)
- [Development Guide](docs/development.md)
- [Troubleshooting](docs/troubleshooting.md)
- [Requirements Specification](REQUIREMENTS_SPECIFICATION.md)
- [Project TODO](TODO.md)

## Sketches

- Main runtime sketch:
  [arduino/robotic_arm/robotic_arm.ino](/home/viktor_srhk/Documents/HighSchool/Personal/SI-CIT/robotic-arm-control/arduino/robotic_arm/robotic_arm.ino)
- Single-servo calibration sketch:
  [arduino/servo_calibration_d5/servo_calibration_d5.ino](/home/viktor_srhk/Documents/HighSchool/Personal/SI-CIT/robotic-arm-control/arduino/servo_calibration_d5/servo_calibration_d5.ino)

## Wokwi And Simulation

- `wokwi.toml` points to `build/robotic_arm.ino.hex` and `build/robotic_arm.ino.elf`
- compile the sketch before launching the simulation so those files exist
- `diagram.json` models the Arduino, breadboard, and servos
- the current Wokwi setup is useful for servo testing, but it does not fully model the physical Nunchuck wiring path

## WiiChuck.h Mode

The default configuration uses the raw I2C reader and does not require an extra library.

If you want to use `WiiChuck.h`:

1. Install the library:

```bash
arduino-cli lib install WiiChuck
```

2. In [robotic_arm.ino](/home/viktor_srhk/Documents/HighSchool/Personal/SI-CIT/robotic-arm-control/arduino/robotic_arm/robotic_arm.ino), change:

```cpp
#define USE_WIICHUCK_LIB 1
```

3. Rebuild the sketch.

If your library version exposes different method names, keep `USE_WIICHUCK_LIB` at `0` or adapt the calls inside `readNunchuck()`.

## Current Limitations

- Final mechanical calibration still depends on the real arm and linkage geometry
- Power quality can still affect smooth motion when multiple servos move together
- The Wokwi diagram is a partial simulation aid, not a full physical replica
- The wrist and gripper tuning constants are software defaults and may need adjustment on real hardware
