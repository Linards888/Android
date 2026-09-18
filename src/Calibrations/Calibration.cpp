#include "Calibration.h"
#include <Arduino.h>

#if Is_IMU
  #include "IMU_logic.h"
#endif

#if Is_blueTooth
  extern void notify(const char* fmt, ...);
#endif

void run_calibration() {
  Serial.println("Calibrating...");

  #if Is_IMU
    Serial.println("Calibrating IMU");
    imu_calibrate();
  #endif

  #if Is_TOF
    Serial.println("TOF sensors: nothing to (re)calibrate here, addresses were assigned at boot.");
  #endif

  #if Is_Sharp
    Serial.println("Sharp sensors use a fixed two-point calibration - see docs/Calibration.md if readings look wrong.");
  #endif

  #if Is_Ultrasonic
    Serial.println("Ultrasonic sensors: nothing to (re)calibrate.");
  #endif

  Serial.println("Calibration complete.");
  #if Is_blueTooth
    notify("calibration complete\n");
  #endif
}
