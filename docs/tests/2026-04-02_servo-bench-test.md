# Test Report

## 1. Test Information

- Date: 02 March 2026
- Author: Viktor
- Test name: Full Servo Bench Test via Serial Commands
- Category:
  - servo bench test
  - full-arm motion test
  - demo rehearsal
  - serial inputs
- Related TODO items: None
- Related files: [/arduino/robotic_arm/robotic_arm.ino](/arduino/robotic_arm/robotic_arm.ino)

## 2. Objective

Watch if Serial ports commands works correctly and if each commands works too.

## 3. Hardware Setup

- Arduino board: Arduino Uno
- Power supply: Arduino board power supply
- Servo(s) used: 6 servos
- Servo pin(s): 3, 5, 6, 9, 10, 11
- External wiring notes: Every servos are connected on the board
- Common ground confirmed: yes

## 4. Software Setup

- Sketch used: [/arduino/robotic_arm/robotic_arm.ino](/arduino/robotic_arm/robotic_arm.ino)
- Commit or version: e637d56d4c1dec7a7f9d89244786e14a651ca34c
- Important constants changed: Fixed commands and angles home, min and max
- Serial monitor baud rate: 115200
- Control mode: serial

## 5. Procedure

Write the exact steps followed.

1. Branch every servo to the correct pins
2. Upload the sketch to the board
3. Test every commands
4. Constats the results

## 6. Measurements And Observations

### Quantitative Data

| Metric | Expected | Measured | Notes |
| --- | --- | --- | --- |
| Voltage | 3.3V | NaN | Fine |
| Current | NaN | NaN | NaN |
| Angle range | Correct | Correct | Fine |
| Duration | NaN | NaN | NaN |
| Temperature | Correct | Correct | Fine |

### Qualitative Observations

- Every commands works well now
- Found no problems on commands
- But the rotation jitter and is saccaded on the wirst

## 7. Test Result

- Result: pass
- Summary: Every serial port commands pass to the test

## 8. Issues Found

- None
- The rotation jitter and is saccaded on the wirst

## 9. Actions To Take

- [ ] Need to find a way that the wirst rotate smoothly
- [x] Maybe add more commands

## 10. Photos / Videos / Evidence

- Photo paths: None
- Video paths: None
- Serial logs: None
- Extra notes: None

---

# Servo Test Add-on

Use this block when testing one motor or one joint.

## Servo Details

- Joint name:
- Servo model:
- Servo type: continuous / positional
- Safe min command:
- Safe max command:
- Neutral / home value:
- Reversed in software: yes / no

## Servo-Specific Checks

| Check | Result | Notes |
| --- | --- | --- |
| Correct pin mapping |  |  |
| Correct direction |  |  |
| Reaches target range |  |  |
| No mechanical collision |  |  |
| No abnormal noise |  |  |
| No overheating |  |  |
| No visible jitter |  |  |

---

# Full Arm Test Add-on

Use this block when testing the whole robotic arm.

## Full-System Checks

| Check | Result | Notes |
| --- | --- | --- |
| 6 servos powered correctly | Yes | NaN |
| Arduino does not reset under load | No | Only when the Nunchuck lib is activated |
| Demo mode runs correctly | Yes | NaN |
| Motion remains smooth for 10+ minutes | NaN | Nan |
