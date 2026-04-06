# Project TODO

Current state: the core control software is implemented, but hardware validation, final calibration, and demo preparation still need to be completed on the real arm.

## Phase 1 - Hardware Validation

- [x] Identify exact servo model and current draw (x3 MG996R + x3 SG90)
- [x] Validate external 5V-6V power supply under load
- [x] Confirm common ground wiring (Arduino + servo PSU + Nunchuck)
- [x] Verify Nunchuck I2C communication stability
- [x] Test one large and one micro servo with external power

## Phase 2 - Core Motion Control

- [x] Wire all 6 servos and validate pin mapping
- [x] Calibrate min/max angle limits for each joint
- [x] Tune smoothing step and update interval
- [x] Add per-servo neutral/home values
- [x] Stress test 10+ minutes continuous movement

## Phase 3 - Wii Nunchuck Control

- [x] Tune joystick deadzone
- [x] Map joystick axes to base/shoulder comfortably
- [x] Validate C/Z button handling for gripper
- [x] Validate hold C+Z for demo mode toggle
- [x] Validate long hold C+Z for home reset

## Phase 4 - Demo For Open Day

- [x] Create stable auto sequence (demo mode)
- [x] Add serial debug output for quick diagnostics
- [ ] Add startup self-check behavior (optional)
- [ ] Rehearse 3-minute demo script

## Phase 5 - Documentation

- [ ] Finalize wiring diagram with exact pin labels
- [ ] Add real setup photos
- [ ] Add short demo video or GIF
- [x] Update README with final controls and limits
- [ ] Prepare oral explanation (architecture + safety)

## Optional Advanced Phase

- [ ] Build desktop Rust CLI sending serial commands
- [x] Define simple serial protocol (ex: `set <joint> <angle>`)
- [ ] Add record/replay movement mode
- [ ] Evaluate embedded Rust prototype separately
