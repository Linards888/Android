#pragma once
#include <Arduino.h>
#include "config.h"

#if Is_IMU
  #include <Wire.h>
  #include <FastIMU.h>

  // imu_cal_data is reset and repopulated by imu_calibrate(); the BLE
  // "calibrate" command (and cmd_start) reference it via `extern`.
  extern calData imu_cal_data;

  void imu_setup();
  void imu_calibrate();          // keep the robot flat & still while this runs
  void imu_update(float dtSeconds);

  // All of these return the value as of the last imu_update() call.
  float imu_pitch();     // degrees, complementary-filtered (accel + gyro)
  float imu_yaw();       // degrees, integrated gyro only - WILL drift, for logging only
  float imu_accel_x();
  float imu_accel_y();
  float imu_accel_z();
  float imu_gyro_x();
  float imu_gyro_y();
  float imu_gyro_z();
#endif
