#pragma once

// ============================================================
//  HardwareEnums.h — symbolic names used by configs/config_*.h
// ============================================================
//  Included once by the root config.h, before your per-robot
//  config file, so that file can write things like:
//
//      #define DRIVE_TOPOLOGY DRIVE_2_MOTOR
//      #define STEERING_MODE  STEERING_DIFFERENTIAL
//      #define MOTOR_KIND     MOTOR_BRUSHED
//
//  instead of bare numbers. These are plain #defines (not a C++
//  enum) so they can be compared with the preprocessor (#if) in
//  src/drive/Drive.cpp — a runtime enum can't be.
// ============================================================

// How many independently-driven propulsion motors the chassis has.
#define DRIVE_1_MOTOR 1
#define DRIVE_2_MOTOR 2
#define DRIVE_4_MOTOR 4

// How the robot turns.
#define STEERING_NONE         0  // can't steer (straight-line testbed / not wired yet)
#define STEERING_DIFFERENTIAL 1  // left/right motors at different speeds (tank/skid steering)
#define STEERING_SERVO        2  // a steering servo, propulsion motor(s) all run the same speed

// What kind of motor driver each MOTOR_LIST entry is.
#define MOTOR_BRUSHED   0  // DC gearmotor through an H-bridge (two pins per motor)
#define MOTOR_BRUSHLESS 1  // brushless motor through an ESC (one PWM signal pin per motor)

// Which side a motor is physically on. Only meaningful for STEERING_DIFFERENTIAL;
// ignored otherwise.
#define SIDE_NONE  0
#define SIDE_LEFT  1
#define SIDE_RIGHT 2

// Distance-sensor "role" — what its reading is used for.
#define ROLE_STEER 0  // feeds the weighted steering-error sum (e.g. left/right wall sensors)
#define ROLE_FRONT 1  // used for speed-scaling and the "too close" escape maneuver
#define ROLE_AUX   2  // read available via dist(name), but not used by the built-in logic

// Sentinel for "this pin isn't used" (e.g. a ToF sensor with no XSHUT wired,
// or the unused second pin of a brushless ESC entry).
#define PIN_NONE 255
