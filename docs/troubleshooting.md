# Troubleshooting

## Power And Resets

- If the Arduino resets when multiple servos move, the servo supply is usually sagging.
- Use a separate external 5V to 6V supply for the servos.
- Make sure all grounds are connected together.
- If the arm behaves erratically at startup, verify the servo supply before increasing motion speed.

## Jitter Or Saccade

- Small wrist jitter usually means the accelerometer input is noisy or the servo is reacting to tiny changes.
- The firmware already uses a deadband, filtered wrist input, and per-joint smoothing.
- If motion still looks choppy, the next things to check are power stability, mechanical binding, and servo mounting.
- If only `wrist roll` and `wrist pitch` look rough together, suspect current draw or supply noise before changing the code again.

## Inverted Motion

- If a control moves the wrong way, first decide whether the problem is logical input mapping or physical servo orientation.
- For joystick and accelerometer axes, the fix is usually in the mapping inside `manualControl()`.
- For a servo mounted backwards, the fix is the `servoReversed` calibration in the main sketch.
- If a command looks right in serial but wrong on the arm, the problem is usually mechanical, not sensor-related.

## Nunchuck Read Failures

- Verify SDA is on `A4` and SCL is on `A5`.
- Check the Nunchuck power and ground wiring.
- If the sketch prints no useful motion, confirm that the controller is actually being read in the selected mode.
- A bad I2C connection usually affects all Nunchuck-driven joints, not just one joint.

## Gripper Speed

- Gripper speed is software-controlled in the main sketch.
- If the gripper feels too slow or too fast, tune the gripper speed constant rather than the angle limits first.
- If the gripper only moves a little, check whether the joint limits are too tight for your exact servo and linkage.

## Calibration Checks

- Use the D5 servo calibration sketch to test one servo at a time.
- Start at `90`, then move in small steps to confirm direction, neutral position, and safe travel.
- If the motion is correct in the calibration sketch but wrong in the main sketch, the issue is likely in the mapping layer rather than the hardware.
