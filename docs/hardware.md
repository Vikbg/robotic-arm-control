# Hardware Guide

## System Overview

The project is an Arduino Uno controlled 6-DOF arm driven by a Wii Nunchuck over I2C and six servos on PWM pins.

The runtime sketch is [arduino/robotic_arm/robotic_arm.ino](../arduino/robotic_arm/robotic_arm.ino).

## Parts And Roles

- `D3` base, continuous-rotation servo
- `D5` shoulder, positional servo
- `D6` elbow, positional servo
- `D9` wrist roll, positional servo
- `D10` wrist pitch, positional servo
- `D11` gripper, positional servo

The code treats the base as a continuous servo command around `90`, while the other joints are managed as angles in the `0..180` range.

## Wiring

### Servos

| Joint | Arduino pin | Notes |
| --- | --- | --- |
| Base | `D3` | Continuous rotation, `90` is stop |
| Shoulder | `D5` | Positional |
| Elbow | `D6` | Positional |
| Wrist roll | `D9` | Positional |
| Wrist pitch | `D10` | Positional |
| Gripper | `D11` | Positional |

### Wii Nunchuck

| Signal | Arduino pin |
| --- | --- |
| SDA | `A4` |
| SCL | `A5` |
| VCC | `3.3V` or `5V`, depending on the module |
| GND | `GND` |

## Power Notes

- Use an external 5V to 6V supply for the servos.
- Keep Arduino logic on USB or a separate regulated source.
- Tie Arduino GND, servo supply GND, and Nunchuck GND together.
- Do not power all servos from the Arduino `5V` pin.

## Simulation Notes

- `wokwi.toml` points to the compiled firmware in `build/`.
- `diagram.json` currently models the Arduino, breadboard, and servos.
- The Wokwi diagram is useful for servo wiring and motion testing, but it does not fully represent the physical Nunchuck wiring path.
