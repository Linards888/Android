#include "sharp_logic.h"
#include "../Utils/ParamRegistry.h"
#include <string.h>

#if SENSOR_SHARP_ENABLED

#define X(name, pin, angle, role, weight) SharpSensor sharp_##name = { { #name, role, weight, 0.0f, false }, pin };
    SHARP_SENSOR_LIST
#undef X

#define X(name, pin, angle, role, weight) &sharp_##name,
    SharpSensor* sharp_all[] = { SHARP_SENSOR_LIST };
#undef X

const uint8_t SHARP_SENSOR_COUNT = sizeof(sharp_all) / sizeof(sharp_all[0]);

static const float FILTER_ALPHA = 0.5f; // higher = less smoothing / more responsive

void sharp_init() {
    for (uint8_t i = 0; i < SHARP_SENSOR_COUNT; i++) {
        pinMode(sharp_all[i]->pin, INPUT);
    }
}

void sharp_update() {
    for (uint8_t i = 0; i < SHARP_SENSOR_COUNT; i++) {
        int raw = analogRead(sharp_all[i]->pin);
        DistReading& r = sharp_all[i]->r;
        r.value_mm = r.valid ? (raw * FILTER_ALPHA + r.value_mm * (1.0f - FILTER_ALPHA)) : (float)raw;
        r.valid = true;
    }
}

DistReading* sharp_find(const char* name) {
    for (uint8_t i = 0; i < SHARP_SENSOR_COUNT; i++) {
        if (strcmp(sharp_all[i]->r.name, name) == 0) return &sharp_all[i]->r;
    }
    return nullptr;
}

void sharp_register_params() {
    // NOTE: the 5th macro parameter is named `w_` here (not `weight`) so it
    // doesn't shadow the literal ".r.weight" member access below — X-macro
    // parameters substitute textually, including inside "r.weight" if the
    // parameter were named the same as that member.
    #define X(name, pin, angle, role, w_) REGISTER_PARAM_NAMED("w_" #name, sharp_##name.r.weight);
        SHARP_SENSOR_LIST
    #undef X
}

#endif
