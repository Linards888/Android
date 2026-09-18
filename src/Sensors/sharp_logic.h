#pragma once
#include <Arduino.h>
#include "config.h"

#if Is_Sharp
  struct SharpSensor {
    const char* name;
    uint8_t pin;
    int16_t angle;
    uint16_t lastReadingMM;
  };

  #define X(name, pin, angle) extern SharpSensor sharp_##name;
    SHARP_SENSOR_LIST
  #undef X

  extern SharpSensor* allSharpSensors[];
  extern const uint8_t SHARP_SENSOR_COUNT;

  void sharp_setup();
  uint16_t sharp_read(SharpSensor* s);
  uint16_t sharp_read(const char* name);
  SharpSensor* sharp_getByName(const char* name);
#endif
