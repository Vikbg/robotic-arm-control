# Embedded Rust Evaluation

## Goal

This note evaluates whether an embedded Rust prototype is a good next step for the robotic arm project.

## Current Situation

The current firmware is written in Arduino C++ and already provides:

- Nunchuck control
- serial diagnostics
- demo mode
- joint limits
- home position
- basic record and replay support

That means the control behavior is already defined well enough to compare another implementation against it.

## What Rust Could Bring

- stronger type safety for control state
- better separation between hardware drivers and motion logic
- easier long-term refactoring if the project grows
- a good learning opportunity for embedded systems programming

## What Rust Would Cost

- more setup effort than Arduino C++
- a steeper toolchain for an Arduino Uno class board
- tighter memory constraints on AVR
- more time spent on infrastructure instead of hardware validation

## Recommendation

For this project, embedded Rust should stay a separate experiment and not replace the demo firmware before the public presentation.

That is the safest path because:

- the Arduino version already works as the main reference implementation
- the real remaining risks are hardware validation and calibration, not language choice
- a Rust prototype could delay the demo without improving the most urgent project risks

## Best Scope For A Separate Prototype

If a prototype is attempted, keep it very small:

1. blink or serial hello-world on the target board
2. drive one servo safely
3. read one simple input source
4. only then attempt multi-servo coordination

## Verdict

- Main demo firmware: keep Arduino C++
- Desktop tooling: Rust is a good fit now
- Embedded Rust: worth exploring later, but as a separate prototype
