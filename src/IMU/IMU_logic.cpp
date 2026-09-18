#include "IMU_logic.h"

#if FEATURE_IMU

#include <FastIMU.h>
#include "../Utils/RobotState.h"
#include "../Utils/Notify.h"

static BMI160 imu;
static calData imu_cal_data = {0};
static AccelData accel_data = {0};
static GyroData gyro_data = {0};

static bool ready = false;
static float pitch_deg = 0.0f;
static float yaw_deg = 0.0f;

void imu_init() {
    int err = imu.init(imu_cal_data, IMU_I2C_ADDRESS);
    if (err != 0) {
        notify("IMU init failed (code %d)\n", err);
        ready = false;
        return;
    }
    ready = true;
}

void imu_recalibrate() {
    if (!ready) return;
    notify("calibrating imu, keep the robot still...\n");
    imu.calibrateAccelGyro(&imu_cal_data);
    imu.init(imu_cal_data, IMU_I2C_ADDRESS);
    pitch_deg = 0.0f;
    yaw_deg = 0.0f;
    notify("imu calibrated\n");
}

bool imu_ready() { return ready; }

void imu_update(float dt_seconds) {
    if (!ready) return;

    imu.update();
    imu.getAccel(&accel_data);
    imu.getGyro(&gyro_data);

    // pitch: complementary filter between gyro-integrated angle (responsive,
    // drifts) and accelerometer tilt (stable, noisy) — state.k_pitch_running
    // is the gyro's trust weight, live-tunable over BLE.
    float pitch_from_accel = atan2f(-accel_data.accelX,
                                     sqrtf(accel_data.accelY * accel_data.accelY +
                                           accel_data.accelZ * accel_data.accelZ)) * RAD_TO_DEG;

    pitch_deg = state.k_pitch_running * (pitch_deg + gyro_data.gyroY * dt_seconds)
              + (1.0f - state.k_pitch_running) * pitch_from_accel;

    // yaw: pure gyro integration. No magnetometer, so this drifts over time —
    // fine for a single run, not for long-term heading tracking.
    yaw_deg += gyro_data.gyroZ * dt_seconds;
}

float imu_pitch() { return pitch_deg; }
float imu_yaw()   { return yaw_deg; }
float imu_accel_x_g() { return accel_data.accelX; }
float imu_accel_y_g() { return accel_data.accelY; }
float imu_accel_z_g() { return accel_data.accelZ; }
float imu_gyro_x_dps() { return gyro_data.gyroX; }
float imu_gyro_y_dps() { return gyro_data.gyroY; }
float imu_gyro_z_dps() { return gyro_data.gyroZ; }

#endif
