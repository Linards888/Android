#pragma once
#include <Arduino.h>
#include "config.h"

#if Is_servo
  // Uses the ESP32Servo library (install via Library Manager: "ESP32Servo").
  // The Setup.md docs previously named a different, unverifiable "kkads_servo"
  // library; ESP32Servo is the standard, well-documented choice for driving
  // a hobby servo from an ESP32 and is what this driver expects.
  void steering_setup();

  // steer: a signed correction value from the drive PID. Positive turns
  // right, negative turns left. Internally clamped to +-SERVO_MAX_DELTA_DEG
  // around SERVO_CENTER_DEG (both set in config.h).
  void steering_set(float steer);
  void steering_center();
#endif
