#pragma once
#include "config.h"


// ---- Error Messages ----

#if (Is_Arduino + Is_Esp32) == 0
  #error "No board selected! Set Is_Arduino to 0 and Is_Esp32 to 1."
#endif

#if (Is_Arduino + Is_Esp32) > 1
  #error "Multiple boards selected! Only one of Is_Arduino / Is_Esp32 can be 1 at a time."
#endif

#if (OneMotor + TwoMotors + tank) == 0
  #error "No motor configuration selected! Choose one of OneMotor / TwoMotors / tank."
#endif

#if (OneMotor + TwoMotors + tank) > 1
  #error "Multiple motor configurations selected! Choose only one of OneMotor / TwoMotors / tank."
#endif

// ---- libraries inclusion ----

#include "Drive.h"

#if Is_IMU
  #include <FastIMU.h>
  #include <Wire.h>
  #include "IMU_logic.h"
#endif

#if Is_TOF
  #include <VL53L0X.h>
  #include <Wire.h>
  #include "tof_logic.h"
#endif

#if Is_blueTooth
  #include <Arduino.h>
  #include <BLEDevice.h>
  #include <BLEUtils.h>
  #include <BLEServer.h>

  #include "notify.h"
  #include "RobotBLE.h"
#endif

#if Is_Ultrasonic
  #include "Ultrasonic_logic.h"
#endif

#if Is_servo
  #include <kkads_servo.h>
#endif

#if Memory
  #include <Preferences.h>
  #include "Memory.h"
#endif