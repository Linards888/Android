#pragma once

// ============================================================
//  config.h — which robot am I building for?
// ============================================================
//  This file is intentionally almost empty. All of the actual
//  per-robot settings (pins, sensors, motors, features) live in
//  their own file under configs/. That way you can keep one
//  config file per physical Folkrace robot you own, and switch
//  between them by changing ONE line below — without copying or
//  branching the rest of the codebase.
//
//  To build for a specific robot:
//    1. Copy configs/config_template.h to configs/config_<yourbot>.h
//       and fill it in (see configs/config_template.h for every
//       option, heavily commented).
//    2. Change ACTIVE_ROBOT_CONFIG below to point at that file.
//    3. Upload. That's it — nothing else in the project changes.
//
//  Everything else (Android.ino, src/*) reads the definitions
//  your active config file provides and adapts itself. See
//  docs/Arhitecture.md for how that works.
// ============================================================

#define ACTIVE_ROBOT_CONFIG "configs/config_example_2motor.h"

#include "src/Utils/HardwareEnums.h"
#include ACTIVE_ROBOT_CONFIG

// ---- sanity checks that apply no matter which robot is active ----

#ifndef ARDUINO_ARCH_ESP32
    #error "This firmware targets ESP32 boards only (uses BLE + Preferences). Select an ESP32 board in Tools > Board."
#endif
