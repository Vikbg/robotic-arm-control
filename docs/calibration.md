# Calibration

This repo has two calibration layers: logical software calibration in `arduino/robotic_arm/robotic_arm.ino`, and a small one-servo test sketch in `arduino/servo_calibration_d5/servo_calibration_d5.ino`.

## Servo Calibration Sketch

`arduino/servo_calibration_d5/servo_calibration_d5.ino` is a diagnostic sketch for the servo on `D5`.

Use it when you want to verify:

- the servo is wired correctly
- the direction is correct
- the neutral or center position is sensible
- the servo responds smoothly before plugging it into the full arm

The sketch uses `90` as the neutral point because that is the usual midpoint for positional servos and the stop point for many continuous servos.

## Main Sketch Calibration

The main runtime sketch uses these calibration values:

- `minAngle[]`
- `maxAngle[]`
- `homeAngle[]`
- `servoReversed[]`
- `JOINT_SMOOTH_STEP[]`
- `WRIST_FILTER_ALPHA`
- `WRIST_TARGET_DEADBAND`
- `GRIPPER_SPEED_PER_SEC`

Only `servoReversed` changes the physical command sent to a servo. The logical angle model stays the same, so the rest of the code can keep using normal joint angles.

## When To Change What

- Change `homeAngle[]` if the arm should rest in a different safe pose.
- Change `minAngle[]` and `maxAngle[]` if a joint hits a mechanical stop or collides with the frame.
- Change `servoReversed[]` if the physical servo is mounted in the opposite direction.
- Change `JOINT_SMOOTH_STEP[]` if a joint feels too abrupt or too sluggish.
- Change `WRIST_FILTER_ALPHA` or `WRIST_TARGET_DEADBAND` if the wrist jitters too much.
- Change `GRIPPER_SPEED_PER_SEC` if the gripper opens or closes too slowly or too quickly.

## Diagnosing Inversion

If a joint moves the wrong way, first check the software convention:

- if the joystick or tilt input is inverted, fix the input mapping
- if the command is correct but the servo motion is reversed, fix `servoReversed[]`

That is the reason the code keeps logical angles separate from the final servo write.
