#pragma once

// ============================================================
//  ROBOT CONFIG TEMPLATE
// ============================================================
//  Copy to configs/config_<your_robot_name>.h, fill in the values
//  for your robot, point config.h's ACTIVE_ROBOT_CONFIG at it.
//  Full explanation of every option: docs/Arhitecture.md.
//  Filled-in examples: config_example_2motor.h, config_example_1motor_servo.h
// ============================================================


// ---- 1) identity ----
#define ROBOT_NAME    "Folkrace"   // BLE advertised name
#define ROBOT_STORAGE "folkrace"   // flash namespace for saved settings (unique per robot)


// ---- 2) optional features — 0 = compiled out, 1 = on ----
#define FEATURE_BLUETOOTH       1   // BLE tuning (src/BLE)
#define FEATURE_IMU             0   // gyro/accel (src/IMU)
#define FEATURE_MEMORY          1   // remember tuned parameters across power cycles (src/Memory)
#define FEATURE_TELEMETRY       0   // CSV stream to a PC over Serial (src/Telemetry)
#define FEATURE_SPACE_AWARENESS 0   // NOT IMPLEMENTED — safe to leave 0 (src/spaceAverenes)


// ---- 3) distance sensors — enable what you physically have ----
#define SENSOR_SHARP_ENABLED      0  // analog IR (e.g. Sharp GP2Y0A21)
#define SENSOR_TOF_ENABLED        0  // VL53L0X (I2C)
#define SENSOR_ULTRASONIC_ENABLED 0  // HC-SR04 style

#define PIN_SDA PIN_NONE   // only needed if SENSOR_TOF_ENABLED or FEATURE_IMU
#define PIN_SCL PIN_NONE

// Format: X(name, pin, angleDeg, role, weight) — see docs/Arhitecture.md
// for what role/weight/angleDeg mean. Sensor names must be unique across
// all three lists below.
#if SENSOR_SHARP_ENABLED
    #define SHARP_SENSOR_LIST \
        X(left,  PIN_NONE, -45, ROLE_STEER, -1.0f) \
        X(right, PIN_NONE,  45, ROLE_STEER,  1.0f)
#endif

// Format: X(name, xshutPin, i2cAddress, angleDeg, role, weight)
// xshutPin: PIN_NONE for a single ToF sensor; give each sensor a real
// xshutPin + unique i2cAddress if running more than one on the bus.
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

// Name of the single ROLE_FRONT sensor that drives speed-scaling / the escape maneuver.
#define PRIMARY_FRONT_SENSOR "front"


// ---- 4) IMU (optional) ----
#if FEATURE_IMU
    #define IMU_I2C_ADDRESS 0x68
#endif


// ---- 5) drive — pick your chassis layout ----
// DRIVE_TOPOLOGY: DRIVE_1_MOTOR | DRIVE_2_MOTOR | DRIVE_4_MOTOR
// STEERING_MODE:  STEERING_NONE | STEERING_DIFFERENTIAL | STEERING_SERVO
// MOTOR_KIND:     MOTOR_BRUSHED | MOTOR_BRUSHLESS
// Full explanation + combination table: docs/Arhitecture.md ("Drive").
#define DRIVE_TOPOLOGY DRIVE_2_MOTOR
#define STEERING_MODE  STEERING_DIFFERENTIAL
#define MOTOR_KIND     MOTOR_BRUSHED

// Format: X(name, side, pinA, pinB)
//   side: SIDE_LEFT / SIDE_RIGHT (STEERING_DIFFERENTIAL only) or SIDE_NONE
//   brushed:   pinA/pinB = the two H-bridge input pins
//   brushless: pinA = ESC signal pin, pinB = PIN_NONE (unused)
#define MOTOR_LIST \
    X(left,  SIDE_LEFT,  PIN_NONE, PIN_NONE) \
    X(right, SIDE_RIGHT, PIN_NONE, PIN_NONE)

// Only needed if STEERING_MODE is STEERING_SERVO.
// #define STEERING_SERVO_PIN PIN_NONE
// #define STEERING_SERVO_CENTER_US 1500  // optional, defaults shown
// #define STEERING_SERVO_RANGE_US  500   // optional, +/- us from center at full lock
