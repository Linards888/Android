#pragma once
#include "Arduino.h"

struct PIDC {
  float& kp;
  float& ki;
  float& kd;

  int previous_error;
  int P;
  float I;
  int D;

  int update(int delta, float DeltaTime){
    P = delta * kp;
    I += delta * DeltaTime;
    D = (delta - previous_error) / DeltaTime;
    int PID = P + I * ki + D * kd;

    return PID;
  }
}