#pragma once
#include <Arduino.h>
#include "config.h"

// ============================================================
//  RobotBLE — everything Bluetooth lives here and only here.
// ============================================================
//  Android.ino calls ble_init() once in setup() (if FEATURE_BLUETOOTH
//  is on) and never touches BLE again. Incoming writes are parsed
//  into "command args" and handed to commands.cpp::handle_command().
//  Outgoing text (command replies, log lines, ...) goes out via
//  src/Utils/Notify.h's notify() — this module just registers
//  itself as Notify's extra sink, so nothing else needs to know
//  BLE exists.
// ============================================================

#if FEATURE_BLUETOOTH

void ble_init();

#endif
