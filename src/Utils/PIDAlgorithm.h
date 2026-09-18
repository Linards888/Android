#pragma once
#include <Arduino.h>

// A small, self-contained PID controller.
//
// Usage:
//   PIDController pid;
//   pid.configure(kp, ki, kd);   // call again any time the gains change
//   pid.reset();                 // call whenever you start a fresh run
//   float output = pid.update(error, dtSeconds);
//
// Integral term is clamped (anti-windup) so a long-lived error can't make
// the output run away. Derivative is computed from consecutive errors, so
// the very first update() after reset() has no derivative contribution.
class PIDController {
public:
  void configure(float kp, float ki, float kd, float integralLimit = 1000.0f) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
    _integralLimit = integralLimit;
  }

  void reset() {
    _integral = 0.0f;
    _prevError = 0.0f;
    _havePrev = false;
  }

  float update(float error, float dtSeconds) {
    if (dtSeconds <= 0.0f) dtSeconds = 0.001f;

    _integral = constrain(_integral + error * dtSeconds, -_integralLimit, _integralLimit);
    float derivative = _havePrev ? (error - _prevError) / dtSeconds : 0.0f;
    _prevError = error;
    _havePrev = true;

    return _kp * error + _ki * _integral + _kd * derivative;
  }

private:
  float _kp = 0.0f;
  float _ki = 0.0f;
  float _kd = 0.0f;
  float _integral = 0.0f;
  float _prevError = 0.0f;
  float _integralLimit = 1000.0f;
  bool _havePrev = false;
};
