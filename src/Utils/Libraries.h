#pragma once
#include "config.h"


// ---- Error Messages ----

#if (DCOneMotor + DCTwoMotors + DCtank + BLDCOneMotor + BLDCTwoMotors + BLDCtank) == 0
  #error "No motor configuration selected! ..."
#endif

#if (DCOneMotor + DCTwoMotors + DCtank + BLDCOneMotor + BLDCTwoMotors + BLDCtank) > 1
  #error "Multiple motor configurations selected! ..."
#endif

#if (DCOneMotor + DCTwoMotors + DCtank + BLDCOneMotor + BLDCTwoMotors + BLDCtank) == 0
  #error "No motor configuration selected! Choose one of DCOneMotor / DCTwoMotors / DCtank / BLDCOneMotor / BLDCTwoMotors / BLDCtank in config.h."
#endif

#if (DCOneMotor + DCTwoMotors + DCtank + BLDCOneMotor + BLDCTwoMotors + BLDCtank) > 1
  #error "Multiple motor configurations selected! Choose only one of DCOneMotor / DCTwoMotors / DCtank / BLDCOneMotor / BLDCTwoMotors / BLDCtank in config.h."
#endif

#if Is_servo && (DCtank || BLDCtank)
  #error "Is_servo doesn't make sense with a tank (4-motor) config - steer with the motors instead. Turn Is_servo off."
#endif

#if spaceControl && !Is_IMU
  #error "spaceControl needs the IMU sensor - set Is_IMU to 1, or turn spaceControl off."
#endif

// ---- libraries inclusion ----

#include "Drive.h"

// Tunable parameters (PID/motor/sensor/debug) + the registry that reads,
// writes, and saves/loads them by name. Not gated behind any feature
// flag - Android.ino and Drive.cpp both use these directly regardless of
// whether BLE is enabled.
#include "Params.h"
#include "ParamRegistry.h"

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

#if Is_Sharp
  #include "sharp_logic.h"
#endif

#if Is_blueTooth
  #include <Arduino.h>
  #include <BLEDevice.h>
  #include <BLEUtils.h>
  #include <BLEServer.h>

  #include "notify.h"
  #include "RobotBLE.h"
  #include "Commands.h"
#endif

#if Is_servo
  #include <kkads_servo.h>
#endif
