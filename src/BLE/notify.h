#pragma once

// printf-style helper that sends text back over the BLE characteristic
// (see RobotBLE.cpp) and mirrors it to Serial, so command replies show
// up whether you're testing over BLE, the Serial monitor, or both.
// Safe to call before BLE is set up - it just won't notify anyone until
// RobotBLE::begin() has run (Serial still gets the message either way).
void notify(const char* fmt, ...);
