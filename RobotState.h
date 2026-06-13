#ifndef ROBOT_STATES_H
#define ROBOT_STATES_H

#include <Arduino.h>

// Define your 6 robot states
enum RobotState {
  STATE_IDLE,
  STATE_CALIBRATION,
  STATE_COUNTDOWN,
  STATE_RUNNING,
  STATE_FORWARD,
  STATE_BACKWARDS
};

// Share the state variable across files
extern RobotState currentState;

#endif