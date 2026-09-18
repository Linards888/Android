#pragma once

// ============================================================
//  EXAMPLE CONFIG — classic RC-car layout: 1 drive motor +
//  1 steering servo, single front VL53L0X, no side sensors.
// ============================================================
//  A second, deliberately different example to show how little
//  changes between robots: this is a completely different
//  chassis from configs/config_example_2motor.h, but Android.ino,
//  every src/ module, and the BLE command set are identical.
//  Only this file (and config.h's ACTIVE_ROBOT_CONFIG line) differ.
// ============================================================


// ---- 1) identity ----
#define ROBOT_NAME    "Folkrace-RC"
#define ROBOT_STORAGE "folkrace_rc"


// ---- 2) optional features ----
#define FEATURE_BLUETOOTH       1
#define FEATURE_IMU             0
#define FEATURE_MEMORY          1
#define FEATURE_TELEMETRY       0
#define FEATURE_SPACE_AWARENESS 0


// ---- 3) distance sensors ----
#define SENSOR_SHARP_ENABLED      0
#define SENSOR_TOF_ENABLED        1
#define SENSOR_ULTRASONIC_ENABLED 0

#define PIN_SDA 8
#define PIN_SCL 9

// A single front ToF, ROLE_FRONT, drives speed-scaling and the escape
// maneuver. With only one sensor there's nothing for the weighted
// steering sum (sensors_steer_error() in Android.ino) to add up — this
// example robot just drives straight and dodges obstacles. Add side
// sensors (ROLE_STEER, like in config_example_2motor.h) for real wall
// following, or replace run_line_following()'s steering line with your
// own logic (e.g. a fixed sweep, or a camera/line-sensor input).
#if SENSOR_TOF_ENABLED
    #define TOF_SENSOR_LIST \
        X(front, PIN_NONE, 0x29, 0, ROLE_FRONT, 0.0f)
#endif

#define PRIMARY_FRONT_SENSOR "front"


// ---- 5) drive ----
#define DRIVE_TOPOLOGY DRIVE_1_MOTOR
#define STEERING_MODE  STEERING_SERVO
#define MOTOR_KIND     MOTOR_BRUSHED

// Format: X(name, side, pinA, pinB) — side is irrelevant with only one
// motor and servo steering, so SIDE_NONE.
#define MOTOR_LIST \
    X(main, SIDE_NONE, 4, 5)

#define STEERING_SERVO_PIN 6
// #define STEERING_SERVO_CENTER_US 1500  // optional, defaults shown in configs/config_template.h
// #define STEERING_SERVO_RANGE_US  500
