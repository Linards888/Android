#pragma once
#include <Arduino.h>
#include "RobotState.h"

// A textbook PID loop. Holds references into RobotState::pid so that when a
// BLE command (or ParamRegistry) changes kp/ki/kd, this controller uses the
// new gains on the very next update() call — no re-wiring needed.
struct PIDController {
    float& kp;
    float& ki;
    float& kd;

    PIDController(PIDState& pid_state)
        : kp(pid_state.kp), ki(pid_state.ki), kd(pid_state.kd) {}

    // error: the current control error (e.g. weighted sensor sum)
    // dt:    seconds since the previous update() call
    int update(float error, float dt) {
        if (dt <= 0.0f) dt = 0.001f; // guard against div-by-zero on the first tick

        integral_ += error * dt;
        integral_ = constrain(integral_, -integral_limit, integral_limit);

        float derivative = (error - previous_error_) / dt;
        previous_error_ = error;

        float output = (kp * error) + (ki * integral_) + (kd * derivative);
        return static_cast<int>(output);
    }

    void reset() {
        previous_error_ = 0.0f;
        integral_ = 0.0f;
    }

    float integral_limit = 100.0f;

private:
    float previous_error_ = 0.0f;
    float integral_ = 0.0f;
};
