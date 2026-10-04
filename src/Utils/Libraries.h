#pragma once
#include "../../config.h"


// ---- Error Messages ----

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

#if Is_IMU
  #error "Is_IMU isn't implemented yet (there is no IMU_logic.h) - set Is_IMU to 0."
#endif

// ---- libraries inclusion ----

#include "../Motors/Drive.h"

// Tunable parameters (PID/motor/sensor/debug) + the registry that reads,
// writes, and saves/loads them by name. Not gated behind any feature
// flag - Android.ino and Drive.cpp both use these directly regardless of
// whether BLE is enabled.
#include "../Params/Params.h"
#include "../Params/ParamRegistry.h"

// Text command protocol - works over Serial even when BLE is off.
#include "../BLE/notify.h"
#include "../BLE/Commands.h"

#if Is_TOF
  #include "../Sensors/tof_logic.h"
#endif

#if Is_Sharp
  #include "../Sensors/sharp_logic.h"
#endif

#if Is_blueTooth
  #include "../BLE/RobotBLE.h"
#endif
