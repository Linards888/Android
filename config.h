#pragma once

/* Change this file for a new car. Put driving/sensor logic in Android.ino. */

// ---- Features -------------------------------------------------------------
#define FOLKRACE_ENABLE_BLE       1
#define FOLKRACE_ENABLE_IMU       0   // Requires the FastIMU library
#define FOLKRACE_ENABLE_SERIAL    1   // USB serial commands for testing

// ---- BLE ------------------------------------------------------------------
#define FOLKRACE_BLE_NAME         "Folkrace"
// Standard Nordic UART Service: supported by nRF Connect and Serial Bluetooth Terminal.

// ---- Motor driver ---------------------------------------------------------
// A two-wheel differential drive. Each motor uses a driver with two inputs.
#define FOLKRACE_MOTOR_COUNT      2
#define FOLKRACE_LEFT_IN1         25
#define FOLKRACE_LEFT_IN2         26
#define FOLKRACE_RIGHT_IN1        27
#define FOLKRACE_RIGHT_IN2        14
#define FOLKRACE_LEFT_REVERSED    0
#define FOLKRACE_RIGHT_REVERSED   0
#define FOLKRACE_MAX_POWER        255
#define FOLKRACE_DEAD_BAND        8

// ---- IMU ------------------------------------------------------------------
#define FOLKRACE_I2C_SDA          21
#define FOLKRACE_I2C_SCL          22
#define FOLKRACE_IMU_ADDRESS      0x68
#define FOLKRACE_IMU_MPU6050      1
#define FOLKRACE_IMU_MPU9250      2
#define FOLKRACE_IMU_TYPE         FOLKRACE_IMU_MPU6050

// ---- Safety/control -------------------------------------------------------
#define FOLKRACE_COMMAND_TIMEOUT_MS  600
#define FOLKRACE_LOOP_PERIOD_MS       10
