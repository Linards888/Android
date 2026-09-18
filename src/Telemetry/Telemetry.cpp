#include "Telemetry.h"

#if FEATURE_TELEMETRY

#include "../Utils/RobotState.h"
#include "../Sensors/DistanceSensors.h"

#if FEATURE_IMU
    #include "../IMU/IMU_logic.h"
#endif

static const unsigned long TELEMETRY_INTERVAL_MS = 100; // 10 Hz
static unsigned long last_ms = 0;
static bool header_sent = false;

void telemetry_init() {
    header_sent = false;
}

void telemetry_update() {
    unsigned long now = millis();
    if (now - last_ms < TELEMETRY_INTERVAL_MS) return;
    last_ms = now;

    if (!header_sent) {
        Serial.println("T,millis,state,dist_front,steer_error,speed_forward,pitch,yaw");
        header_sent = true;
    }

    Serial.print("T,");
    Serial.print(now);
    Serial.print(',');
    Serial.print(running_state_name(state.running_state));
    Serial.print(',');
    Serial.print(dist_front());
    Serial.print(',');
    Serial.print(sensors_steer_error());
    Serial.print(',');
    Serial.print(state.speed_forward);
    Serial.print(',');
#if FEATURE_IMU
    Serial.print(imu_pitch());
    Serial.print(',');
    Serial.print(imu_yaw());
#else
    Serial.print("0,0");
#endif
    Serial.println();
}

#endif
