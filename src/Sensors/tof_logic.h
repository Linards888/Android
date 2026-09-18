#pragma once
#include <Arduino.h>
#include <VL53L0X.h>
#include "config.h"
#include "DistanceSensors.h"

// Internal — Android.ino should use src/Sensors/DistanceSensors.h (dist(name))
// instead of this file directly.

#if SENSOR_TOF_ENABLED

struct TofSensor {
    DistReading r;
    VL53L0X sensor;
    uint8_t xshutPin;   // PIN_NONE if this is the only ToF sensor on the bus
    uint8_t address;
};

#define X(name, xshutPin, addr, angle, role, weight) extern TofSensor tof_##name;
    TOF_SENSOR_LIST
#undef X

extern TofSensor* tof_all[];
extern const uint8_t TOF_SENSOR_COUNT;

void tof_init();
void tof_update();
DistReading* tof_find(const char* name);
void tof_register_params(); // registers "w_<name>" for every sensor's steering weight

#endif
