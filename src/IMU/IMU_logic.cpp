#include "IMU_logic.h"

#if FOLKRACE_ENABLE_IMU
#include <Wire.h>
#include <FastIMU.h>

#if FOLKRACE_IMU_TYPE == FOLKRACE_IMU_MPU6050
static MPU6050 imu;
#elif FOLKRACE_IMU_TYPE == FOLKRACE_IMU_MPU9250
static MPU9250 imu;
#else
  #error "Select a supported FOLKRACE_IMU_TYPE in config.h"
#endif

static calData calibration = {};
static bool ready = false;

bool imuBegin() {
  Wire.begin(FOLKRACE_I2C_SDA, FOLKRACE_I2C_SCL);
  const int error = imu.init(calibration, FOLKRACE_IMU_ADDRESS);
  ready = (error == 0);
  if (!ready) Serial.printf("IMU init failed: %d\\n", error);
  return ready;
}

bool imuUpdate(IMUReading& reading) {
  if (!ready || !imu.update()) return false;
  AccelData accel;
  GyroData gyro;
  imu.getAccel(&accel);
  imu.getGyro(&gyro);
  reading.ax = accel.accelX; reading.ay = accel.accelY; reading.az = accel.accelZ;
  reading.gx = gyro.gyroX;   reading.gy = gyro.gyroY;   reading.gz = gyro.gyroZ;
  reading.timestamp = millis();
  return true;
}
bool imuIsReady() { return ready; }
#else
bool imuBegin() { return false; }
bool imuUpdate(IMUReading&) { return false; }
bool imuIsReady() { return false; }
#endif
