#pragma once

// ============================================================
//  Notify — a tiny printf-style logger, decoupled from BLE.
// ============================================================
//  Any module can call notify("kp %.4f\n", state.pid.kp) without
//  knowing or caring whether BLE is enabled. It always echoes to
//  Serial. If BLE is enabled and connected, src/BLE/RobotBLE.cpp
//  registers itself as an extra output sink at boot, so the same
//  message also goes out over the BLE characteristic. This is the
//  mechanism that keeps BLE plumbing out of every other file.
// ============================================================

typedef void (*NotifySink)(const char* message);

void notify(const char* fmt, ...);

// Registers an additional output sink (e.g. BLE). Pass nullptr to clear it.
void notify_set_sink(NotifySink sink);
