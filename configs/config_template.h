#pragma once

// ============================================================
//  ROBOT CONFIG TEMPLATE
// ============================================================
//  Copy this file to configs/config_<your_robot_name>.h, fill in
//  the values for the robot you actually built, then point
//  config.h's ACTIVE_ROBOT_CONFIG at it.
//
//  This is the ONLY file (besides Android.ino's "quick tunables"
//  block) most people ever need to edit. Pins, which sensors you
//  have, what motors/steering you have, and which optional
//  features are on — all live here.
//
//  See configs/config_example_2motor.h for a filled-in example.
// ============================================================


// ------------------------------------------------------------
//  1) IDENTITY
// ------------------------------------------------------------
// Name this robot advertises over Bluetooth, and the flash
// namespace its saved settings live under (keep this unique per
// physical robot if they ever share a Preferences partition).
#define ROBOT_NAME    "Folkrace"
#define ROBOT_STORAGE "folkrace"


// ------------------------------------------------------------
//  2) OPTIONAL FEATURES — 0 = compiled out entirely, 1 = on
// ------------------------------------------------------------
#define FEATURE_BLUETOOTH       1   // BLE tuning/telemetry characteristic (see src/BLE)
#define FEATURE_IMU             0   // gyro/accel (slope detection, yaw tracking)
#define FEATURE_MEMORY          1   // remember tuned parameters across power cycles
#define FEATURE_TELEMETRY       0   // stream state to a PC over Serial (see src/Telemetry)
#define FEATURE_SPACE_AWARENESS 0   // NOT IMPLEMENTED YET — track mapping / path planning.
                                     // Safe to leave at 0. See src/spaceAverenes/space.h
                                     // for what's already scaffolded for this.


// ------------------------------------------------------------
//  3) DISTANCE SENSORS — enable the kinds you physically have
// ------------------------------------------------------------
#define SENSOR_SHARP_ENABLED      0  // analog IR distance sensors (e.g. Sharp GP2Y0A21)
#define SENSOR_TOF_ENABLED        0  // VL53L0X time-of-flight sensors (I2C)
#define SENSOR_ULTRASONIC_ENABLED 0  // HC-SR04 style ultrasonic sensors

// I2C bus pins — only needed if SENSOR_TOF_ENABLED or FEATURE_IMU is 1.
#define PIN_SDA PIN_NONE
#define PIN_SCL PIN_NONE

// Sensor names must be unique across ALL of SHARP_SENSOR_LIST /
// TOF_SENSOR_LIST / USONIC_SENSOR_LIST combined — dist("front") only
// works unambiguously if only one sensor, of any type, is named "front".
//
// Every sensor you list below becomes readable in your own logic as:
//     float d = dist("front");     // millimeters, filtered
// regardless of what physical sensor type it actually is.
//
// `role` says what the built-in RUNNING logic in Android.ino uses it for:
//   ROLE_STEER — feeds the weighted steering-error sum (e.g. left/right walls)
//   ROLE_FRONT — drives speed-scaling + the "too close" escape maneuver
//                (exactly one sensor should normally be ROLE_FRONT — see
//                PRIMARY_FRONT_SENSOR below)
//   ROLE_AUX   — read via dist("name") in your own code, ignored by the
//                built-in logic
//
// `weight` is the ROLE_STEER sensor's contribution to the steering error:
//   error = sum(weight_i * dist_i) for every ROLE_STEER sensor.
// Positive weight steers right, negative steers left, when that sensor
// sees a closer reading. Weights are live-tunable over BLE as "w_<name>".
//
// angleDeg is just metadata for your own reference / future space-awareness
// use (0 = front, 90 = right, -90 = left, 180 = rear) — the built-in logic
// does not currently use it.

// Format: X(name, pin, angleDeg, role, weight)
#if SENSOR_SHARP_ENABLED
    #define SHARP_SENSOR_LIST \
        X(left,  PIN_NONE, -45, ROLE_STEER, -1.0f) \
        X(right, PIN_NONE,  45, ROLE_STEER,  1.0f)
#endif

// Format: X(name, xshutPin, i2cAddress, angleDeg, role, weight)
// xshutPin: PIN_NONE if you only have one ToF sensor (no address conflict
// to resolve). List two or more and give each a real xshutPin + a unique
// i2cAddress if you're running multiple VL53L0X on one bus.
#if SENSOR_TOF_ENABLED
    #define TOF_SENSOR_LIST \
        X(front, PIN_NONE, 0x30, 0, ROLE_FRONT, 0.0f)
#endif

// Format: X(name, echoPin, trigPin, angleDeg, role, weight)
#if SENSOR_ULTRASONIC_ENABLED
    #define USONIC_SENSOR_LIST \
        X(left_side,  PIN_NONE, PIN_NONE, -90, ROLE_AUX, 0.0f) \
        X(right_side, PIN_NONE, PIN_NONE,  90, ROLE_AUX, 0.0f)
#endif

// Which single sensor name drives speed-scaling / the escape maneuver.
// Must match the `name` of exactly one ROLE_FRONT sensor above.
#define PRIMARY_FRONT_SENSOR "front"


// ------------------------------------------------------------
//  4) IMU (optional)
// ------------------------------------------------------------
#if FEATURE_IMU
    #define IMU_I2C_ADDRESS 0x68
#endif


// ------------------------------------------------------------
//  5) DRIVE — pick your chassis layout
// ------------------------------------------------------------
// How many propulsion motors:      DRIVE_1_MOTOR | DRIVE_2_MOTOR | DRIVE_4_MOTOR
// How the robot turns:             STEERING_NONE | STEERING_DIFFERENTIAL | STEERING_SERVO
// What kind of motor driver:       MOTOR_BRUSHED | MOTOR_BRUSHLESS
//
// Common combinations:
//   - 1 motor + steering servo (classic RC car layout):
//       DRIVE_TOPOLOGY = DRIVE_1_MOTOR, STEERING_MODE = STEERING_SERVO
//   - 2 motors, no servo -> tank/skid steering:
//       DRIVE_TOPOLOGY = DRIVE_2_MOTOR, STEERING_MODE = STEERING_DIFFERENTIAL
//   - 2 motors + steering servo (both drive, servo steers):
//       DRIVE_TOPOLOGY = DRIVE_2_MOTOR, STEERING_MODE = STEERING_SERVO
//   - 4 motors, no servo -> 4-wheel differential (like a bigger tank):
//       DRIVE_TOPOLOGY = DRIVE_4_MOTOR, STEERING_MODE = STEERING_DIFFERENTIAL
//   - 4 motors + steering servo:
//       DRIVE_TOPOLOGY = DRIVE_4_MOTOR, STEERING_MODE = STEERING_SERVO
//   - Any of the above with brushless motors + ESCs instead of brushed
//     DC gearmotors: just set MOTOR_KIND to MOTOR_BRUSHLESS. Each
//     MOTOR_LIST entry's second pin is then unused (pass PIN_NONE) since
//     an ESC only needs one PWM signal wire.

#define DRIVE_TOPOLOGY DRIVE_2_MOTOR
#define STEERING_MODE  STEERING_DIFFERENTIAL
#define MOTOR_KIND     MOTOR_BRUSHED

// Format: X(name, side, pinA, pinB)
//   side: SIDE_LEFT / SIDE_RIGHT (used by STEERING_DIFFERENTIAL) or SIDE_NONE
//   brushed:   pinA/pinB = the two H-bridge input pins for that motor
//   brushless: pinA = ESC signal pin, pinB = PIN_NONE
#define MOTOR_LIST \
    X(left,  SIDE_LEFT,  PIN_NONE, PIN_NONE) \
    X(right, SIDE_RIGHT, PIN_NONE, PIN_NONE)

// Only needed if STEERING_MODE is STEERING_SERVO.
// #define STEERING_SERVO_PIN PIN_NONE
// #define STEERING_SERVO_CENTER_US 1500  // optional, defaults shown
// #define STEERING_SERVO_RANGE_US  500   // optional, +/- us from center at full lock
