#pragma once
#include <Arduino.h>
#include "../../config.h"

struct Motor { const char* name; uint8_t in1; uint8_t in2; bool reversed; };
extern Motor* allmotors[];
extern const uint8_t MOTOR_COUNT;

#if TwoMotors
extern Motor motor_left;
extern Motor motor_right;
#endif

void driveBegin();
void setMotor(Motor& motor, int power);
void setMotor(uint8_t index, int power);
void driveTank(int leftPower, int rightPower);
void driveArcade(int throttle, int steering);
void stopMotors();
