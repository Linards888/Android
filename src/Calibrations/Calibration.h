#pragma once
#include "config.h"

// Runs once when the robot enters Mode::CALIBRATION (via the BLE "calibrate"
// command), then Android.ino returns it to Mode::IDLE. Currently this just
// (re)calibrates the IMU bias, if one is enabled - distance sensors don't
// need a runtime calibration step beyond the address assignment already
// done in sensors_setup().
void run_calibration();
