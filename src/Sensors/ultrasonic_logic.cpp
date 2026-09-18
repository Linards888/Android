#include "ultrasonic_logic.h"
#include "../Utils/ParamRegistry.h"
#include <string.h>

#if SENSOR_ULTRASONIC_ENABLED

#define X(name, echoPin, trigPin, angle, role, weight) \
    UsonicSensor usonic_##name = { { #name, role, weight, 0.0f, false }, echoPin, trigPin };
    USONIC_SENSOR_LIST
#undef X

#define X(name, echoPin, trigPin, angle, role, weight) &usonic_##name,
    UsonicSensor* usonic_all[] = { USONIC_SENSOR_LIST };
#undef X

const uint8_t USONIC_SENSOR_COUNT = sizeof(usonic_all) / sizeof(usonic_all[0]);

static const float FILTER_ALPHA = 0.5f;
static const unsigned long ECHO_TIMEOUT_US = 25000UL; // ~4.3m round trip, plenty for Folkrace

void usonic_init() {
    for (uint8_t i = 0; i < USONIC_SENSOR_COUNT; i++) {
        pinMode(usonic_all[i]->trigPin, OUTPUT);
        pinMode(usonic_all[i]->echoPin, INPUT);
        digitalWrite(usonic_all[i]->trigPin, LOW);
    }
}

static float read_one_mm(UsonicSensor* s) {
    digitalWrite(s->trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(s->trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(s->trigPin, LOW);

    unsigned long duration_us = pulseIn(s->echoPin, HIGH, ECHO_TIMEOUT_US);
    if (duration_us == 0) return -1.0f; // timed out / nothing in range

    // speed of sound ~343 m/s -> ~0.343 mm/us round trip, so divide by 2 for one-way
    return (duration_us * 0.343f) / 2.0f;
}

void usonic_update() {
    for (uint8_t i = 0; i < USONIC_SENSOR_COUNT; i++) {
        float raw = read_one_mm(usonic_all[i]);
        if (raw < 0) continue; // keep last good value on a timeout

        DistReading& r = usonic_all[i]->r;
        r.value_mm = r.valid ? (raw * FILTER_ALPHA + r.value_mm * (1.0f - FILTER_ALPHA)) : raw;
        r.valid = true;
    }
}

DistReading* usonic_find(const char* name) {
    for (uint8_t i = 0; i < USONIC_SENSOR_COUNT; i++) {
        if (strcmp(usonic_all[i]->r.name, name) == 0) return &usonic_all[i]->r;
    }
    return nullptr;
}

void usonic_register_params() {
    #define X(name, echoPin, trigPin, angle, role, w_) REGISTER_PARAM_NAMED("w_" #name, usonic_##name.r.weight);
        USONIC_SENSOR_LIST
    #undef X
}

#endif
