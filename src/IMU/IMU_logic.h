#pragma once
#include <Arduino.h>
#include "../../config.h"

struct IMUReading {
  float ax, ay, az;       // g
  float gx, gy, gz;       // degrees/second
  unsigned long timestamp;
};

bool imuBegin();
bool imuUpdate(IMUReading& reading);
bool imuIsReady();
