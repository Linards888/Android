#pragma once
#include <Arduino.h>
#include "config.h"

// Sharp (analog IR) distance sensor handling.
//
// Usage:
//   Sharp::setupAll();                    // once, in setup()
//   uint16_t d = Sharp::read(SharpFront);  // anywhere after that, for whichever sensor you want
//
// The sensors themselves (SharpFront, SharpRight, SharpLeft, SharpRightSide,
// SharpLeftSide) are defined in config.h - edit pins/angles there, not here.

namespace Sharp {
  // Configures the analog input pin for every sensor listed in config.h.
  // Call this once from setup().
  void setupAll();

  // Reads one sensor and returns the distance in millimeters.
  // Pass one of the sensor globals from config.h, e.g. Sharp::read(SharpFront).
  // Returns 65535 (out-of-range / error) if that sensor isn't one of the
  // sensors configured in config.h.
  uint16_t read(const SharpSensor &sensor);
}
