#pragma once
#include <Arduino.h>
#include "config.h"

// ToF (VL53L0X) sensor handling.
//
// Usage:
//   Tof::setupAll();               // once, in setup()
//   uint16_t d = Tof::read(Front); // anywhere after that, for whichever sensor you want
//
// The sensors themselves (Front, Right, Left, RightSide, LeftSide) are
// defined in config.h - edit pins/addresses/angles there, not here.

namespace Tof {
  // Brings every sensor listed in config.h up on the shared I2C bus and
  // gives each one its configured address. Call this once from setup(),
  // after Wire is otherwise free to use.
  void setupAll();

  // Reads one sensor and returns the distance in millimeters.
  // Pass one of the sensor globals from config.h, e.g. Tof::read(Front).
  // Returns 65535 (out-of-range / error) if that sensor failed to init
  // during setupAll() or isn't one of the configured sensors.
  uint16_t read(const TOFSensor &sensor);
}