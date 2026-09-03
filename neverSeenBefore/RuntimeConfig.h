#pragma once

// This intentionally reuses the project's existing feature switches, pin lists,
// and sensor lists. Configure those in ../config.h before uploading.
#include "../config.h"

#if !Is_Esp32 || Is_Arduino
#error "This completed firmware targets an ESP32. Set Is_Esp32=1 and Is_Arduino=0 in config.h."
#endif

#if (OneMotor + TwoMotors + tank) != 1
#error "Select exactly one motor layout in config.h: OneMotor, TwoMotors, or tank."
#endif

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint16_t DEFAULT_COUNTDOWN_MS = 5000;
constexpr uint16_t CONTROL_INTERVAL_MS = 20;
constexpr uint16_t TELEMETRY_INTERVAL_MS = 250;
constexpr uint16_t FRONT_STOP_DISTANCE_MM = 160;
constexpr int MOTOR_PWM_MAX = 255;
