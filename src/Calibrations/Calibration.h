#pragma once
#include <Arduino.h>
#include "config.h"

// ============================================================
//  Calibration — a live sensor dump you run before a race.
// ============================================================
//  This is a diagnostic tool, not magic auto-tuning: while the
//  robot is in RunningState::CALIBRATING it prints every enabled
//  sensor's live reading every ~200ms for a few seconds. Move the
//  robot near/far from walls (or hold it at your desired "near"
//  and "far" reference distances) and watch the numbers over BLE
//  or Serial to pick good dist_near/dist_far/escape_trigger_mm
//  and steering weights — then set them with `set <name> <value>`.
//
//  If FEATURE_IMU is on, entering calibration also re-runs the
//  IMU's gyro/accel calibration (robot must be held still & level
//  for that part).
// ============================================================

void calibration_start();

// Call every loop tick while running_state == CALIBRATING.
// Returns true once calibration has finished (caller should then
// move on to RunningState::READY or IDLE).
bool calibration_run();
