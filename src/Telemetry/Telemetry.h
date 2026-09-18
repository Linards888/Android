#pragma once
#include <Arduino.h>
#include "config.h"

// ============================================================
//  Telemetry — streams robot state to a PC over Serial.
// ============================================================
//  Only compiled in when FEATURE_TELEMETRY is 1. This is the MVP
//  transport: plain CSV lines over the USB-serial port, one every
//  TELEMETRY_INTERVAL_MS, that any PC script (Python + pyserial,
//  a serial plotter, Excel via a terminal capture, ...) can log
//  or graph. A prefixed "T," lets a receiving script tell a
//  telemetry row apart from an ordinary debug/notify line sharing
//  the same serial port.
//
//  This is intentionally simple. If you build the BLE-radio /
//  ground-station telemetry described in the README's roadmap
//  later, it can live alongside this (or replace it) without
//  touching any other module — nothing else depends on Telemetry.
// ============================================================

#if FEATURE_TELEMETRY

void telemetry_init();

// Call once per loop tick — internally rate-limits itself to
// TELEMETRY_INTERVAL_MS, so it's safe (and expected) to call every tick.
void telemetry_update();

#endif
