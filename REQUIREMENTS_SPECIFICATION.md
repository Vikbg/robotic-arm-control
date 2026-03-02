# Software Requirements Specification (SRS)
## 6-DOF Robotic Arm Control System

## 1. Introduction

### 1.1 Purpose
This document defines the functional and non-functional requirements for the 6-DOF Robotic Arm Control System developed as a final SI-CIT project.

### 1.2 Scope
This system enables manual control of a robotic arm composed of six servomotors using an Arduino Uno microcontroller and a Wii Nunchuck interface (joystick + C/Z buttons).

An optional advanced implementation with Rust tooling may be developed if time permits.

## 2. Overall Description

### 2.1 Product Perspective
The system consists of:
- Arduino-based control unit
- Six servo actuators
- External power supply
- Manual input device (Wii Nunchuck on I2C)

The system operates as a standalone embedded control solution.

### 2.2 Product Functions
The system shall:
- Control six servomotors independently
- Receive input from a Wii Nunchuck interface
- Generate PWM signals for servo positioning
- Maintain stable power distribution
- Provide a demonstration mode
- Provide a safe home position

### 2.3 Constraints
- 6 weeks development period
- Limited hardware budget
- Public demonstration requirement
- 5V-6V power constraints

## 3. Functional Requirements

### FR-1 Servo Control
The system shall control 6 servomotors via PWM signals.

### FR-2 Independent Movement
Each servo shall be independently addressable.

### FR-3 Manual Control
The system shall allow real-time control via Wii Nunchuck joystick input.

### FR-4 Demo Mode
The system shall include an automated movement sequence.

### FR-5 Safety Reset
The system shall provide a default safe position.

### FR-6 Mode Toggle
The system shall allow toggling between manual mode and demo mode using Nunchuck buttons.

## 4. Non-Functional Requirements

### NFR-1 Reliability
The system must operate continuously for at least 10 minutes without failure.

### NFR-2 Safety
The system shall use an external power supply for servo motors.

### NFR-3 Stability
Voltage drop must not reset the Arduino.

### NFR-4 Maintainability
The code shall be modular and documented.

### NFR-5 Control Stability
Control input shall include deadzone and smoothing to reduce jitter.

## 5. Future Enhancements

- Rust embedded implementation
- Bluetooth control
- Web interface
- Inverse kinematics
- Position feedback sensors

## 6. Acceptance Criteria

The project will be considered complete if:

- All 6 servos respond correctly to Nunchuck input.
- Nunchuck buttons trigger demo mode and reset reliably.
- The system runs stably during public demonstration.
- Electrical connections are safe and documented.
