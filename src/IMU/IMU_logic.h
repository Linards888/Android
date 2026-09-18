#pragma once
#include <Arduino.h>
#include "config.h"

// ============================================================
//  IMU — gyro/accel wrapper (FastIMU + BMI160).
// ============================================================
//  Only compiled in when FEATURE_IMU is 1. Gives you simple
//  one-line accessors instead of raw sensor math scattered
//  through your control loop:
//
//      float p = imu_pitch();   // degrees
//      float y = imu_yaw();     // degrees, drifts over time (no magnetometer)
//
//  Shares the I2C bus with any ToF sensors. Wire.begin(PIN_SDA,
//  PIN_SCL) is called once, centrally, in Android.ino's setup() —
//  not in here — so this module and src/Sensors/tof_logic.cpp
//  never fight over who initializes the bus.
// ============================================================

#if FEATURE_IMU
    #ifndef IMU_I2C_ADDRESS
        #error "FEATURE_IMU is on but IMU_I2C_ADDRESS isn't defined in your config."
    #endif

void imu_init();

// Call once per control-loop tick, after Wire/sensors have been serviced.
void imu_update(float dt_seconds);

// Restarts gyro/accel calibration (robot must be held still & level).
// Mirrors the old firmware's "recalibrate on every `start` command" behaviour;
// call it from wherever you want that same behaviour (see Android.ino).
void imu_recalibrate();

bool imu_ready();

float imu_pitch();       // degrees, complementary-filtered (see k_pitch_running)
float imu_yaw();         // degrees, gyro-integrated (drifts — fine for short runs)
float imu_accel_x_g();
float imu_accel_y_g();
float imu_accel_z_g();
float imu_gyro_x_dps();
float imu_gyro_y_dps();
float imu_gyro_z_dps();

#endif
