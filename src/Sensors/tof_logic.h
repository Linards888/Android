#pragma once
#include <Arduino.h>
#include "config.h"

#if Is_TOF
  struct tofSensor {
    VL53L0X sensor;
    const char* name;
    uint8_t xshutPin;
    uint8_t address;
    int16_t angle;
  };

  #define X(name, pin, addr, angle) extern tofSensor tof_##name;
    TOF_SENSOR_LIST
  #undef X

  extern TofSensor* allTofSensors[];
  extern const uint8_t TOF_SENSOR_COUNT;

  void tof_setup();
  void tof_readAll();
  uint16_t tof_read(TofSensor* s);        // read just one sensor, by pointer
  uint16_t tof_read(const char* name);    // read just one sensor, by name
  TofSensor* tof_getByName(const char* name);
#endif