#pragma once
#include <Arduino.h>

// ============================================================
//  Every tunable value the robot uses, in one place.
// ------------------------------------------------------------
//  Each one is a plain global, so it can be:
//    - read or changed directly from Android.ino (or anywhere else
//      that includes this header) like any other variable,
//    - read or changed live over BLE/Serial (see ParamRegistry.h,
//      Commands.h) by name,
//    - saved to / loaded from flash with ParamRegistry::save()/load().
//  Defaults live in Params.cpp - change them there, not here.
//
//  Adding a new tunable: declare it here, define+default it in
//  Params.cpp, then add one line to the table in ParamRegistry.cpp so
//  BLE/Serial/flash all pick it up.
// ============================================================

// ---- PID ----
extern float kp, ki, kd;
extern float kleft, kright;
extern int maxdelta, constrainpid, constraindelta;

// ---- Drive speeds ----
extern int maxspeed, minspeed, fspeed, rspeed;

// ---- Sensor thresholds ----
extern int errorleftdist, errorrightdist, minwalldistFront;

// ---- Motor / actuator tuning (consumed by src/Motors/Drive.cpp) ----
extern int DRIVE_MAX;

extern int PWM_FREQ_HZ;
extern int PWM_RESOLUTION_BITS;

extern int ESC_MIN_US;
extern int ESC_NEUTRAL_US;
extern int ESC_MAX_US;

extern int STEER_MIN_DEG;
extern int STEER_CENTER_DEG;
extern int STEER_MAX_DEG;

extern int SERVO_PULSE_MIN_US;
extern int SERVO_PULSE_MAX_US;

// ---- Debug toggles ----
// Nothing here does anything by itself yet - these are just switches +
// intervals for whatever logging you wire up in Android.ino/loop(), kept
// here so they're readable/settable/saveable the same way as everything
// else.
struct DebugState {
  bool logDistance = false;
  uint16_t logDistanceIntervalMs = 200;

  bool logSensors = false;
  uint16_t logSensorsIntervalMs = 200;

  bool logPid = false;
  uint16_t logPidIntervalMs = 200;

  bool bleTelemetry = false;
  uint16_t bleTelemetryIntervalMs = 250;
};
extern DebugState debugState;
