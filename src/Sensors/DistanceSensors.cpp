#include "DistanceSensors.h"
#include "sharp_logic.h"
#include "tof_logic.h"
#include "ultrasonic_logic.h"
#include <string.h>

void sensors_init() {
#if SENSOR_SHARP_ENABLED
    sharp_init();
#endif
#if SENSOR_TOF_ENABLED
    tof_init();
#endif
#if SENSOR_ULTRASONIC_ENABLED
    usonic_init();
#endif
}

void sensors_update() {
#if SENSOR_SHARP_ENABLED
    sharp_update();
#endif
#if SENSOR_TOF_ENABLED
    tof_update();
#endif
#if SENSOR_ULTRASONIC_ENABLED
    usonic_update();
#endif
}

void sensors_register_params() {
#if SENSOR_SHARP_ENABLED
    sharp_register_params();
#endif
#if SENSOR_TOF_ENABLED
    tof_register_params();
#endif
#if SENSOR_ULTRASONIC_ENABLED
    usonic_register_params();
#endif
}

static DistReading* find_any(const char* name) {
    DistReading* r = nullptr;
#if SENSOR_SHARP_ENABLED
    if (!r) r = sharp_find(name);
#endif
#if SENSOR_TOF_ENABLED
    if (!r) r = tof_find(name);
#endif
#if SENSOR_ULTRASONIC_ENABLED
    if (!r) r = usonic_find(name);
#endif
    return r;
}

float dist(const char* name) {
    DistReading* r = find_any(name);
    if (!r || !r->valid) return -1.0f;
    return r->value_mm;
}

float dist_front() {
    return dist(PRIMARY_FRONT_SENSOR);
}

float sensors_steer_error() {
    float sum = 0.0f;

#if SENSOR_SHARP_ENABLED
    for (uint8_t i = 0; i < SHARP_SENSOR_COUNT; i++) {
        DistReading& r = sharp_all[i]->r;
        if (r.valid && r.role == ROLE_STEER) sum += r.weight * r.value_mm;
    }
#endif
#if SENSOR_TOF_ENABLED
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
        DistReading& r = tof_all[i]->r;
        if (r.valid && r.role == ROLE_STEER) sum += r.weight * r.value_mm;
    }
#endif
#if SENSOR_ULTRASONIC_ENABLED
    for (uint8_t i = 0; i < USONIC_SENSOR_COUNT; i++) {
        DistReading& r = usonic_all[i]->r;
        if (r.valid && r.role == ROLE_STEER) sum += r.weight * r.value_mm;
    }
#endif

    return sum;
}
