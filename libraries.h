#pragma once
#include "config.h"
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