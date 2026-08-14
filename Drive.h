#pragma once
#include <Arduino.h>
#include "config.h"

struct Motor {
  const char* name;
  uint8_t MotorPinA;
  uint8_t MotorPinB;
};

#define X(name, MotorPinA, MotorPinB) extern Motor motor_##name;
  MOTOR_LIST
#undef X

extern Motor* allmotors[];
extern const uint8_t MOTOR_COUNT;

void motorsetup();
void MotorDrive(Motor* m, int speed);
void stopMotors();