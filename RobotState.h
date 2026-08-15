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

#endif