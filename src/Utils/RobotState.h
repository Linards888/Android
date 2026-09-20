#ifndef ROBOT_STATES_H
#define ROBOT_STATES_H

#include <Arduino.h>

// Define your 7 robot states
enum RobotState {
  IDLE,
  READY,
  CALIBRATION,
  COUNTDOWN,
  RUNNING,
  FORWARD,
  BACKWARDS
};

// Share the state variable across files
extern RobotState currentState;

// millis() timestamp COUNTDOWN was entered at. Shared so commands (see
// src/BLE/Commands.cpp's "start") can kick off a countdown the same way
// loop()'s own state machine does.
extern unsigned long countdownStartTime;

#endif
