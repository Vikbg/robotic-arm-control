# Project TODO

Current state: the core control software is implemented, but real-arm validation, documented test results, final calibration, and demo preparation still need to be completed.

## Phase 1 - Hardware Validation

- [ ] Document measured servo current draw (x3 MG996R + x3 SG90)
- [ ] Validate external 5V-6V power supply under load on the real arm
- [x] Confirm common ground wiring (Arduino + servo PSU + Nunchuck)
- [ ] Verify Nunchuck I2C communication stability on the real arm
- [ ] Document one large-servo and one micro-servo bench test with external power

## Phase 2 - Core Motion Control

- [x] Wire all 6 servos and document pin mapping
- [ ] Finalize min/max angle limits for each joint on the real arm
- [x] Tune smoothing step and update interval in firmware
- [x] Add per-servo neutral/home values
- [ ] Stress test 10+ minutes continuous movement on the real arm

## Phase 3 - Wii Nunchuck Control

- [x] Tune joystick deadzone
- [x] Map joystick axes to base/shoulder comfortably
- [ ] Validate C/Z button handling for gripper on the real arm
- [ ] Validate hold C+Z for demo mode toggle on the real arm
- [ ] Validate long hold C+Z for home reset on the real arm

## Phase 4 - Demo For Open Day

- [x] Create auto sequence (demo mode)
- [x] Add serial debug output for quick diagnostics
- [x] Add startup self-check behavior (optional)
- [ ] Rehearse 3-minute demo script

## Phase 5 - Documentation

- [x] Finalize wiring diagram with exact pin labels
- [ ] Write hardware test reports from bench and real-arm sessions
- [ ] Add real setup photos
- [ ] Add short demo video or GIF
- [x] Update README with final controls and limits
- [x] Prepare oral explanation (architecture + safety)

## Optional Advanced Phase

- [x] Build desktop Rust CLI sending serial commands
- [x] Define simple serial protocol (ex: `set <joint> <angle>`)
- [x] Add record/replay movement mode
- [x] Evaluate embedded Rust prototype separately
