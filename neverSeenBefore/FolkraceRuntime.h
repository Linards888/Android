#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "Drive.h"
#include "PIDAlgorithm.h"
#include "RobotState.h"
#include "TofLogic.h"

class FolkraceRuntime {
 public:
  void begin();
  void update();
  void handleCommand(String command);

 private:
  void setState(RobotState state);
  void save();
  void telemetry() const;
  void runControl(float dtSeconds);
  uint16_t frontDistance() const;

  RobotState state_ = RobotState::IDLE;
  PIDAlgorithm pid_;
  Preferences preferences_;
  int speed_ = 120;
  uint32_t stateStartedAt_ = 0;
  uint32_t lastControlAt_ = 0;
  uint32_t lastTelemetryAt_ = 0;
  bool tofAvailable_ = false;
};
