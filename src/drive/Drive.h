#pragma once
#include <Arduino.h>
#include "config.h"

// A DC motor driven through two PWM-capable pins (e.g. TB6612, DRV8833-style
// driver): PWM on PinA + LOW on PinB drives forward, PWM on PinB + LOW on
// PinA drives reverse, both LOW coasts.
struct Motor {
  const char* name;
  uint8_t MotorPinA;
  uint8_t MotorPinB;
};

#define X(name, MotorPinA, MotorPinB) extern Motor motor_##name;
  MOTOR_LIST
#undef X

extern Motor* allMotors[];
extern const uint8_t MOTOR_COUNT;

void motorsetup();

// Low-level: drive one motor directly. speed: -255..255. 0 coasts (does not brake).
// You normally don't need to call this yourself - use drive_apply() below,
// which also handles steering, speed ramping and driving in reverse.
void MotorDrive(Motor* m, int speed);
void stopMotors();


// ---- High-level drive control ----
// This is what Android.ino calls each control loop tick. It turns a desired
// forward speed and a steering correction into whatever the robot actually
// has (servo, differential motors, or a single motor), while also:
//   - ramping speed changes using state.accel / state.brake (config.h
//     DEFAULT_ACCEL_STEP / DEFAULT_BRAKE_STEP), so speed doesn't jump
//     instantly and stress the drivetrain or lose grip.
//   - flipping everything if state.drive_reversed is set (the BLE
//     "reverse_drive" command), for when the motors are wired backwards.
//
// forwardSpeed: desired speed, roughly -255..255.
// steer:        signed correction from the steering PID. Positive turns
//               right, negative turns left. Ignored (no effect) if you have
//               neither a servo nor a two-sided drive to steer with.
void drive_apply(int forwardSpeed, float steer);

// Stops the robot immediately and clears the speed ramp and any
// in-progress maneuver. Call this whenever leaving Mode::RUNNING.
void drive_reset();


// ---- Front-wall reverse+turn / manual "180" maneuver ----
// A short, timed sequence - back up, then pivot toward whichever side has
// more room - used when the front sensor sees a dead end, or on the BLE
// "180" debug command. It's non-blocking (no delay()): call
// maneuver_service() once per loop() tick for as long as maneuver_active()
// is true, and it drives the motors itself via drive_apply().
bool maneuver_active();
void maneuver_service();
void maneuver_start_reverse_turn(int16_t leftDist, int16_t rightDist);
void maneuver_start_180();
