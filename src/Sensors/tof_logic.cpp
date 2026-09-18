#include "tof_logic.h"
#include "../Utils/Notify.h"
#include "../Utils/ParamRegistry.h"
#include <string.h>

#if SENSOR_TOF_ENABLED

#define X(name, xshutPin, addr, angle, role, weight) \
    TofSensor tof_##name = { { #name, role, weight, 0.0f, false }, VL53L0X(), xshutPin, addr };
    TOF_SENSOR_LIST
#undef X

#define X(name, xshutPin, addr, angle, role, weight) &tof_##name,
    TofSensor* tof_all[] = { TOF_SENSOR_LIST };
#undef X

const uint8_t TOF_SENSOR_COUNT = sizeof(tof_all) / sizeof(tof_all[0]);

static const float FILTER_ALPHA = 0.5f;

void tof_init() {
    // Only sensors with a real xshutPin need the "hold low, wake one at a
    // time, reassign address" dance — that's only necessary when more than
    // one VL53L0X shares the bus. A lone sensor (xshutPin == PIN_NONE) is
    // just initialized directly at its default address.
    bool any_xshut = false;
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
        if (tof_all[i]->xshutPin != PIN_NONE) {
            any_xshut = true;
            pinMode(tof_all[i]->xshutPin, OUTPUT);
            digitalWrite(tof_all[i]->xshutPin, LOW);
        }
    }
    if (any_xshut) delay(10);

    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
        TofSensor* s = tof_all[i];

        if (s->xshutPin != PIN_NONE) {
            digitalWrite(s->xshutPin, HIGH);
            delay(10);
        }

        if (!s->sensor.init()) {
            notify("TOF sensor '%s' failed to init\n", s->r.name);
            continue;
        }

        if (s->xshutPin != PIN_NONE) {
            s->sensor.setAddress(s->address);
        }

        s->sensor.setTimeout(50);
        s->sensor.startContinuous();
    }
}

void tof_update() {
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
        TofSensor* s = tof_all[i];
        uint16_t raw = s->sensor.readRangeContinuousMillimeters();

        if (s->sensor.timeoutOccurred()) {
            // keep the last good value rather than smoothing in a bogus reading
            continue;
        }

        DistReading& r = s->r;
        r.value_mm = r.valid ? (raw * FILTER_ALPHA + r.value_mm * (1.0f - FILTER_ALPHA)) : (float)raw;
        r.valid = true;
    }
}

DistReading* tof_find(const char* name) {
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
        if (strcmp(tof_all[i]->r.name, name) == 0) return &tof_all[i]->r;
    }
    return nullptr;
}

void tof_register_params() {
    #define X(name, xshutPin, addr, angle, role, w_) REGISTER_PARAM_NAMED("w_" #name, tof_##name.r.weight);
        TOF_SENSOR_LIST
    #undef X
}

#endif
