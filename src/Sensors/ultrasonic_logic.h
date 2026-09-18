#pragma once
#include <Arduino.h>
#include "config.h"

#if Is_Ultrasonic
  struct UltrasonicSensor {
    const char* name;
    uint8_t trigPin;
    uint8_t echoPin;
    int16_t angle;
    uint16_t lastReadingMM;
  };

  #define X(name, trigPin, echoPin, angle) extern UltrasonicSensor usonic_##name;
    USONIC_SENSOR_LIST
  #undef X

  extern UltrasonicSensor* allUsonicSensors[];
  extern const uint8_t USONIC_SENSOR_COUNT;

  void ultrasonic_setup();
  uint16_t ultrasonic_read(UltrasonicSensor* s);
  uint16_t ultrasonic_read(const char* name);
  UltrasonicSensor* ultrasonic_getByName(const char* name);
#endif
