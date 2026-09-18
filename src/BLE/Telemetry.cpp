#include "Telemetry.h"

#if Is_blueTooth
#include "RobotBLE.h"
#include "RobotState.h"
#include "Sensors.h"

#if Is_IMU
  #include "IMU_logic.h"
#endif

static unsigned long lastLogDistance = 0, lastLogGyro = 0, lastLogAccel = 0, lastLogYaw = 0, lastLogPitch = 0;

void service_debug_logs() {
  unsigned long now = millis();

  if (state.debug.log_distance && now - lastLogDistance >= state.debug.log_distance_interval_ms) {
    lastLogDistance = now;
    notify("dist f:%d l:%d r:%d\n", sensor_read("front"), sensor_read("left"), sensor_read("right"));
  }

#if Is_IMU
  if (state.debug.log_gyro && now - lastLogGyro >= state.debug.log_gyro_interval_ms) {
    lastLogGyro = now;
    notify("gyro x:%.2f y:%.2f z:%.2f\n", imu_gyro_x(), imu_gyro_y(), imu_gyro_z());
  }
  if (state.debug.log_accel && now - lastLogAccel >= state.debug.log_accel_interval_ms) {
    lastLogAccel = now;
    notify("accel x:%.2f y:%.2f z:%.2f\n", imu_accel_x(), imu_accel_y(), imu_accel_z());
  }
  if (state.debug.log_yaw && now - lastLogYaw >= state.debug.log_yaw_interval_ms) {
    lastLogYaw = now;
    notify("yaw %.2f\n", imu_yaw());
  }
  if (state.debug.log_pitch && now - lastLogPitch >= state.debug.log_pitch_interval_ms) {
    lastLogPitch = now;
    notify("pitch %.2f\n", imu_pitch());
  }
#endif
}

#endif
