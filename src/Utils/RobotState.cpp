#include "RobotState.h"

RobotState state;

const char* running_state_name(RunningState s) {
    switch (s) {
        case RunningState::IDLE:            return "IDLE";
        case RunningState::READY:           return "READY";
        case RunningState::CALIBRATING:     return "CALIBRATING";
        case RunningState::COUNTDOWN:       return "COUNTDOWN";
        case RunningState::RUNNING:         return "RUNNING";
        case RunningState::MANUAL_FORWARD:  return "MANUAL_FORWARD";
        case RunningState::MANUAL_BACKWARD: return "MANUAL_BACKWARD";
    }
    return "UNKNOWN";
}
