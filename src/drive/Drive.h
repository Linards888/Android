#pragma once
#include <Arduino.h>
#include "config.h"

// A DC motor driven through two PWM-capable pins (e.g. TB6612, DRV8833-style
// driver): PWM on PinA + LOW on PinB drives forward, PWM on PinB + LOW on
// PinA drives reverse, both LOW coasts.
struct Motor {
  const char* name;
  uint8_t MotorPinA;
  uint8_t MotorPinB;
};

#define X(name, MotorPinA, MotorPinB) extern Motor motor_##name;
  MOTOR_LIST
#undef X

extern Motor* allMotors[];
extern const uint8_t MOTOR_COUNT;

void motorsetup();

// speed: -255..255. 0 coasts (does not brake).
void MotorDrive(Motor* m, int speed);
void stopMotors();
