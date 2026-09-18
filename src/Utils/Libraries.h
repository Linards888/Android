#pragma once
#include "config.h"


// ---- Configuration sanity checks ----
// These turn a wrong/missing config.h selection into a compile-time error
// instead of confusing runtime behaviour.

#if Is_Arduino == 1
  #error "turn off Is_Arduino - this firmware needs ESP32 features (BLE, Preferences, PWM) and won't work on plain Arduino boards."
#endif

#if (Is_Arduino + Is_Esp32) == 0
  #error "No board selected! Set Is_Esp32 to 1 in config.h."
#endif

#if (Is_Arduino + Is_Esp32) > 1
  #error "Multiple boards selected! Only one of Is_Arduino / Is_Esp32 can be 1 at a time."
#endif

#if (OneMotor + TwoMotors + tank) == 0
  #error "No motor configuration selected! Choose one of OneMotor / TwoMotors / tank in config.h."
#endif

#if (OneMotor + TwoMotors + tank) > 1
  #error "Multiple motor configurations selected! Choose only one of OneMotor / TwoMotors / tank in config.h."
#endif

#if Is_vl53l8cx
  #error "Is_vl53l8cx is not implemented yet - leave it at 0 in config.h."
#endif

#if Telemetry
  #error "Telemetry is not implemented yet - leave it at 0 in config.h."
#endif

#if spaceControl
  #error "spaceControl is not implemented yet - leave it at 0 in config.h."
#endif

#if (Is_Sharp + Is_TOF + Is_Ultrasonic) == 0
  #warning "No distance sensor is enabled in config.h - sensor_read() will have nothing to read."
#endif

#if OneMotor && !Is_servo
  #warning "OneMotor without Is_servo has no way to steer - drive_apply()'s steer argument will be ignored."
#endif


// ---- Library / module includes ----
// Only pull in what the enabled features actually need.

#include "Drive.h"
#include "RobotState.h"
#include "PIDAlgorithm.h"
#include "DeltaTime.h"
#include "Sensors.h"

#if Is_IMU
  #include "IMU_logic.h"
#endif

#if Is_blueTooth
  #include <Arduino.h>
  #include <BLEDevice.h>
  #include <BLEUtils.h>
  #include <BLEServer.h>

  #include "RobotBLE.h"
  #include "commands.h"
  #include "Telemetry.h"
#endif

#if Is_servo
  #include "Steering.h"
#endif

#if Memory
  #include <Preferences.h>
  #include "Memory.h"
#endif

#include "Calibration.h"
