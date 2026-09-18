#pragma once
#include "config.h"

#if Is_blueTooth
  // Sends whichever "log ..." debug streams (distance/gyro/accel/yaw/pitch)
  // are turned on and due, over BLE via notify(). Toggled and timed by the
  // BLE "log" command (see src/BLE/commands.cpp). Call once per loop().
  void service_debug_logs();
#endif
