# Test Report Template

Use this template to document one test session at a time.

Suggested filename:

`docs/tests/YYYY-MM-DD_short-test-name.md`

Example:

`docs/tests/2026-04-06_servo-bench-test.md`

---

# Test Report

## 1. Test Information

- Date:
- Author:
- Test name:
- Category:
  - servo bench test
  - power test
  - Nunchuck input test
  - full-arm motion test
  - demo rehearsal
- Related TODO items:
- Related files:

## 2. Objective

Describe exactly what this test is supposed to confirm.

Example:

- verify that the shoulder servo moves smoothly from `100` to `160`
- measure whether the power supply stays stable when 3 servos move together
- confirm that `C + Z` toggles demo mode reliably

## 3. Hardware Setup

- Arduino board:
- Power supply:
- Nunchuck voltage:
- Servo(s) used:
- Servo pin(s):
- External wiring notes:
- Common ground confirmed: yes / no

## 4. Software Setup

- Sketch used:
- Commit or version:
- Important constants changed:
- Serial monitor baud rate:
- Control mode:

## 5. Procedure

Write the exact steps followed.

1. 
2. 
3. 
4. 

## 6. Measurements And Observations

### Quantitative Data

| Metric | Expected | Measured | Notes |
| --- | --- | --- | --- |
| Voltage |  |  |  |
| Current |  |  |  |
| Angle range |  |  |  |
| Duration |  |  |  |
| Temperature |  |  |  |

### Qualitative Observations

- 
- 
- 

## 7. Test Result

- Result: pass / partial / fail
- Summary:

## 8. Issues Found

- 
- 

## 9. Actions To Take

- [ ] 
- [ ] 
- [ ] 

## 10. Photos / Videos / Evidence

- Photo paths:
- Video paths:
- Serial logs:
- Extra notes:

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
| 6 servos powered correctly |  |  |
| Arduino does not reset under load |  |  |
| Nunchuck I2C remains stable |  |  |
| Button gestures work reliably |  |  |
| Demo mode runs correctly |  |  |
| Home reset works correctly |  |  |
| Motion remains smooth for 10+ minutes |  |  |
