# Project TODO

## Phase 1 - Hardware Validation

- [ ] Identify exact servo model and current draw (x3 MG996R + x3 SG90)
- [ ] Validate external 5V-6V power supply under load
- [ ] Confirm common ground wiring (Arduino + servo PSU + Nunchuck)
- [ ] Verify Nunchuck I2C communication stability
- [ ] Test one large and one micro servo with external power

## Phase 2 - Core Motion Control

- [ ] Wire all 6 servos and validate pin mapping
- [ ] Calibrate min/max angle limits for each joint
- [ ] Tune smoothing step and update interval
- [ ] Add per-servo neutral/home values
- [ ] Stress test 10+ minutes continuous movement

## Phase 3 - Wii Nunchuck Control

- [ ] Tune joystick deadzone
- [ ] Map joystick axes to base/shoulder comfortably
- [ ] Validate C/Z button handling for gripper
- [ ] Validate hold C+Z for demo mode toggle
- [ ] Validate long hold C+Z for home reset

## Phase 4 - Demo For Open Day

- [ ] Create stable auto sequence (demo mode)
- [ ] Add serial debug output for quick diagnostics
- [ ] Add startup self-check behavior (optional)
- [ ] Rehearse 3-minute demo script

## Phase 5 - Documentation

- [ ] Finalize wiring diagram with exact pin labels
- [ ] Add real setup photos
- [ ] Add short demo video or GIF
- [ ] Update README with final controls and limits
- [ ] Prepare oral explanation (architecture + safety)

## Optional Advanced Phase

- [ ] Build desktop Rust CLI sending serial commands
- [ ] Define simple serial protocol (`SET joint angle`)
- [ ] Add record/replay movement mode
- [ ] Evaluate embedded Rust prototype separately
