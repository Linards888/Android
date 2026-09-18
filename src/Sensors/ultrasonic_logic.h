#pragma once
#include <Arduino.h>
#include "config.h"
#include "DistanceSensors.h"

// Internal — Android.ino should use src/Sensors/DistanceSensors.h (dist(name))
// instead of this file directly.

#if SENSOR_ULTRASONIC_ENABLED

struct UsonicSensor {
    DistReading r;
    uint8_t echoPin;
    uint8_t trigPin;
};

#define X(name, echoPin, trigPin, angle, role, weight) extern UsonicSensor usonic_##name;
    USONIC_SENSOR_LIST
#undef X

extern UsonicSensor* usonic_all[];
extern const uint8_t USONIC_SENSOR_COUNT;

void usonic_init();
void usonic_update();
DistReading* usonic_find(const char* name);
void usonic_register_params(); // registers "w_<name>" for every sensor's steering weight

#endif
