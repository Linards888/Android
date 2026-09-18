#include "IMU_logic.h"

#if Is_IMU

static IMU_MODEL IMU(Wire);
calData imu_cal_data = { 0 };

static AccelData accel = { 0 };
static GyroData gyro = { 0 };
static float pitch_deg = 0.0f;
static float yaw_deg = 0.0f;

void imu_setup() {
  Wire.begin();
  int err = IMU.init(imu_cal_data, IMU_addres);
  if (err != 0) {
    Serial.print("IMU init failed, error code: ");
    Serial.println(err);
  }
}

// Run this with the robot flat and completely still. Overwrites imu_cal_data
// with freshly measured accel/gyro bias, then re-inits the IMU so every
// reading after this call has the bias already removed.
void imu_calibrate() {
  Serial.println("Calibrating IMU - keep the robot flat and still...");
  IMU.calibrateAccelGyro(&imu_cal_data);
  IMU.init(imu_cal_data, IMU_addres);
  Serial.println("IMU calibration done.");
}

void imu_update(float dtSeconds) {
  IMU.update();
  IMU.getAccel(&accel);
  IMU.getGyro(&gyro);

  // Complementary filter: fuse the accelerometer's noisy-but-drift-free tilt
  // angle with the smooth-but-drifting integrated gyro rate. Good enough for
  // "are we going up a ramp" - not a substitute for a real AHRS.
  float accelPitch = atan2(-accel.accelX, sqrtf(accel.accelY * accel.accelY + accel.accelZ * accel.accelZ)) * RAD_TO_DEG;
  const float alpha = 0.98f;
  if (dtSeconds > 0.0f) {
    pitch_deg = alpha * (pitch_deg + gyro.gyroY * dtSeconds) + (1.0f - alpha) * accelPitch;
    // Yaw has no absolute reference without a magnetometer, so this is a
    // plain gyro integration and WILL drift over time. Fine for a debug
    // telemetry log, not for navigation.
    yaw_deg += gyro.gyroZ * dtSeconds;
  } else {
    pitch_deg = accelPitch;
  }
}

float imu_pitch()   { return pitch_deg; }
float imu_yaw()     { return yaw_deg; }
float imu_accel_x() { return accel.accelX; }
float imu_accel_y() { return accel.accelY; }
float imu_accel_z() { return accel.accelZ; }
float imu_gyro_x()  { return gyro.gyroX; }
float imu_gyro_y()  { return gyro.gyroY; }
float imu_gyro_z()  { return gyro.gyroZ; }

#endif
