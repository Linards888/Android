#include "Calibration.h"
#include "../Sensors/DistanceSensors.h"
#include "../Sensors/sharp_logic.h"
#include "../Sensors/tof_logic.h"
#include "../Sensors/ultrasonic_logic.h"
#include "../Utils/Notify.h"
#include "../Utils/RobotState.h"

#if FEATURE_IMU
    #include "../IMU/IMU_logic.h"
#endif

static unsigned long calib_start_ms = 0;
static unsigned long last_print_ms = 0;

static const unsigned long CALIBRATION_DURATION_MS = 3000;
static const unsigned long CALIBRATION_PRINT_INTERVAL_MS = 200;

void calibration_start() {
    calib_start_ms = millis();
    last_print_ms = 0;

    notify("calibrating: watching sensors for %lu ms\n", CALIBRATION_DURATION_MS);

#if FEATURE_IMU
    if (state.imu_enabled) {
        imu_recalibrate();
    }
#endif
}

static void print_readings() {
#if SENSOR_SHARP_ENABLED
    for (uint8_t i = 0; i < SHARP_SENSOR_COUNT; i++) {
        notify("  %s: %.0f\n", sharp_all[i]->r.name, sharp_all[i]->r.value_mm);
    }
#endif
#if SENSOR_TOF_ENABLED
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
        notify("  %s: %.0f mm\n", tof_all[i]->r.name, tof_all[i]->r.value_mm);
    }
#endif
#if SENSOR_ULTRASONIC_ENABLED
    for (uint8_t i = 0; i < USONIC_SENSOR_COUNT; i++) {
        notify("  %s: %.0f mm\n", usonic_all[i]->r.name, usonic_all[i]->r.value_mm);
    }
#endif
}

bool calibration_run() {
    unsigned long now = millis();
    sensors_update();

    if (now - last_print_ms >= CALIBRATION_PRINT_INTERVAL_MS) {
        last_print_ms = now;
        print_readings();
    }

    if (now - calib_start_ms >= CALIBRATION_DURATION_MS) {
        notify("calibration done\n");
        return true;
    }
    return false;
}
