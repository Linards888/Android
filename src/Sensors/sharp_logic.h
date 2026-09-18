#pragma once
#include <Arduino.h>
#include "config.h"
#include "DistanceSensors.h"

// Internal — Android.ino should use src/Sensors/DistanceSensors.h (dist(name))
// instead of this file directly.

#if SENSOR_SHARP_ENABLED

struct SharpSensor {
    DistReading r;
    uint8_t pin;
};

#define X(name, pin, angle, role, weight) extern SharpSensor sharp_##name;
    SHARP_SENSOR_LIST
#undef X

extern SharpSensor* sharp_all[];
extern const uint8_t SHARP_SENSOR_COUNT;

void sharp_init();
void sharp_update();
DistReading* sharp_find(const char* name);
void sharp_register_params(); // registers "w_<name>" for every sensor's steering weight

#endif
