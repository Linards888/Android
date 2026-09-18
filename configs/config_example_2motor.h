#pragma once

// ============================================================
//  EXAMPLE CONFIG — 2 brushed motors, tank steering,
//  2x Sharp IR (left/right wall following) + 1x VL53L0X (front)
// ============================================================
//  This mirrors the very first working Folkrace robot this
//  project was built around (ESP32-C3 "mini" board). Copy this
//  file as a starting point for a similarly-wired robot, or use
//  configs/config_template.h for a from-scratch build with a
//  different sensor/motor layout.
// ============================================================


// ---- 1) identity ----
#define ROBOT_NAME    "Folkrace"
#define ROBOT_STORAGE "folkrace"


// ---- 2) optional features ----
#define FEATURE_BLUETOOTH       1
#define FEATURE_IMU             0   // this robot has a BMI160 wired up but it's unused for now —
                                     // flip to 1 and see src/IMU/IMU_logic.cpp once you want slope/yaw logic
#define FEATURE_MEMORY          1
#define FEATURE_TELEMETRY       0
#define FEATURE_SPACE_AWARENESS 0


// ---- 3) distance sensors ----
#define SENSOR_SHARP_ENABLED      1
#define SENSOR_TOF_ENABLED        1
#define SENSOR_ULTRASONIC_ENABLED 0

#define PIN_SDA 4
#define PIN_SCL 3

// Format: X(name, pin, angleDeg, role, weight)
#if SENSOR_SHARP_ENABLED
    #define SHARP_SENSOR_LIST \
        X(left,  1, -45, ROLE_STEER, -1.0f) \
        X(right, 0,  45, ROLE_STEER,  1.0f)
#endif

// Format: X(name, xshutPin, i2cAddress, angleDeg, role, weight)
// Only one ToF sensor on the bus -> no xshut/address dance needed (PIN_NONE).
#if SENSOR_TOF_ENABLED
    #define TOF_SENSOR_LIST \
        X(front, PIN_NONE, 0x29, 0, ROLE_FRONT, 0.0f)
#endif

#define PRIMARY_FRONT_SENSOR "front"


// ---- 4) IMU ----
#if FEATURE_IMU
    #define IMU_I2C_ADDRESS 0x68
#endif


// ---- 5) drive ----
#define DRIVE_TOPOLOGY DRIVE_2_MOTOR
#define STEERING_MODE  STEERING_DIFFERENTIAL
#define MOTOR_KIND     MOTOR_BRUSHED

// Format: X(name, side, pinA, pinB)
#define MOTOR_LIST \
    X(left,  SIDE_LEFT,  7, 10) \
    X(right, SIDE_RIGHT, 6, 5)

// no steering servo on this robot (STEERING_MODE is STEERING_DIFFERENTIAL)
