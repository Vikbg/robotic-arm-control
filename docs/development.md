# Development Guide

## Tooling

The project uses `arduino-cli` with the AVR core for Arduino Uno.

```bash
arduino-cli config init
arduino-cli core update-index
arduino-cli core install arduino:avr
```

If you keep a local copy in `./bin/arduino-cli`, use that path in place of `arduino-cli`.

## Build And Upload

From the repository root:

```bash
arduino-cli compile --fqbn arduino:avr:uno arduino/robotic_arm
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno arduino/robotic_arm
arduino-cli monitor -p /dev/ttyACM0 -c baudrate=115200
```

Useful discovery commands:

```bash
arduino-cli board list
arduino-cli board listall
```

## Wokwi Workflow

- `wokwi.toml` points Wokwi at `build/robotic_arm.ino.hex` and `build/robotic_arm.ino.elf`.
- Compile the sketch before starting the simulation so those artifacts exist.
- The generated files live under `build/` and are not source of truth.
- If the simulation does not reflect your latest code, rebuild first.

## Source Files

- Runtime sketch: [arduino/robotic_arm/robotic_arm.ino](../arduino/robotic_arm/robotic_arm.ino)
- Calibration sketch: [arduino/servo_calibration_d5/servo_calibration_d5.ino](../arduino/servo_calibration_d5/servo_calibration_d5.ino)
