#pragma once
#include <Arduino.h>

enum RobotState : uint8_t { IDLE, READY, COUNTDOWN, RUNNING };
extern RobotState currentState;
