# Control System

This project is driven by `arduino/robotic_arm/robotic_arm.ino`, which supports four modes:

- `MODE_NUNCHUCK` for live control from the Wii Nunchuck
- `MODE_SERIAL` for direct diagnostics and manual jogs over USB serial
- `MODE_DEMO` for the automatic motion sequence
- `MODE_REPLAY` for replaying a recorded movement sequence

The sketch keeps a logical angle model in software, then writes physical servo commands at the end. That matters because one joint can be mechanically reversed without changing the rest of the control logic.

## Joint Map

The six joints are mapped to these pins:

- `D3` base
- `D5` shoulder
- `D6` elbow
- `D9` wrist roll
- `D10` wrist pitch
- `D11` gripper

The base is a continuous-rotation servo, so it is driven with a stop-centered command around `90`. The other joints are positional servos and use angle limits.

## Calibration Values

Current logical home angles are:

- base `90`
- shoulder `130`
- elbow `90`
- wrist roll `90`
- wrist pitch `90`
- gripper `60`

Current limits are:

- shoulder `100..160`
- elbow `0..160`
- wrist roll `0..180`
- wrist pitch `20..160`
- gripper `15..75`

The code also contains these tuning values:

- `WRIST_FILTER_ALPHA = 0.05`
- `WRIST_TARGET_DEADBAND = 3`
- `WRIST_TARGET_RATE_LIMIT = 2`
- `GRIPPER_SPEED_PER_SEC = 35.0`
- `JOINT_SMOOTH_STEP = {0, 2, 2, 1, 1, 2}`

`servoReversed[WRIST_PITCH]` is enabled, so wrist pitch is reversed only at write time. That keeps the logical angles consistent everywhere else.

## Nunchuck Mapping

The control mapping is:

- joystick X -> base speed
- joystick Y -> shoulder in layer A, elbow in layer B
- tilt X -> wrist roll
- tilt Y -> wrist pitch
- C tap -> switch between layer A and layer B
- C held -> open gripper
- Z held -> close gripper
- C + Z held about `0.7s` -> toggle demo mode
- C + Z held about `1.8s` -> home position and return to Nunchuck mode

The Nunchuck accelerometer values are filtered before mapping so the wrist does not jitter as much when both wrist axes move together.

## Serial Mode

Serial control is also part of the runtime behavior. The supported commands are:

- `h` or `help`
- `p`
- `pins`
- `selfcheck`
- `mode s|n|d`
- `sel <0-5>`
- `set <joint> <angle>`
- `step <joint> <delta>`
- `rec start|stop|clear|status`
- `play`
- `stop`

Single-key shortcuts are also supported:

- `1..6` select a joint
- `a` and `z` jog the selected joint by `-2` or `+2`
- `m`, `n`, and `d` switch or report mode

## Runtime Flow

The loop is intentionally simple:

1. Read serial input.
1. Read the Nunchuck unless serial or replay mode is active.
1. Resolve gestures and control mode.
1. Compute target angles.
1. Optionally record the current target frame.
1. Apply one smooth movement step to each servo.

That separation is why the comments in the sketch focus on calibration and control intent instead of line-by-line mechanics.
