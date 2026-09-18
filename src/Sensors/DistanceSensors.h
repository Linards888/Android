#pragma once
#include <Arduino.h>
#include "config.h"

// ============================================================
//  DistanceSensors — the "recall the distance from a sensor
//  with one simple call" abstraction.
// ============================================================
//  This is the ONLY sensor header Android.ino (or your own
//  RUNNING-state logic) should ever need to include. It doesn't
//  matter whether a given named sensor is physically a Sharp IR,
//  a VL53L0X ToF, or an ultrasonic — from your code it's just:
//
//      float front = dist("front");
//      float error = sensors_steer_error();   // pre-weighted left/right sum
//
//  Which physical sensors exist, and what each one is named, is
//  entirely decided by your configs/config_*.h SHARP_SENSOR_LIST /
//  TOF_SENSOR_LIST / USONIC_SENSOR_LIST. Adding a sensor means
//  adding one line there — nothing here changes.
// ============================================================

// One named distance reading. Shared shape for every sensor type.
struct DistReading {
    const char* name;
    uint8_t role;      // ROLE_STEER / ROLE_FRONT / ROLE_AUX (see HardwareEnums.h)
    float weight;       // this sensor's contribution to sensors_steer_error()
    float value_mm;      // last filtered reading. For Sharp IR this is a smoothed
                          // *raw ADC count*, not a true millimeter distance, unless
                          // you add your own linearization — see sharp_logic.cpp.
    bool valid;           // false until the first successful reading
};

void sensors_init();
void sensors_update();   // call once per control-loop tick, before reading anything

// Registers every configured sensor's steering weight as a BLE/save-able
// param named "w_<sensor name>" (e.g. "w_left"). Call once from setup(),
// alongside your own REGISTER_PARAM calls.
void sensors_register_params();

// Look up any configured sensor by name. Returns -1 if the name is unknown
// or that sensor category is disabled in config.
float dist(const char* name);

// Shortcut for dist(PRIMARY_FRONT_SENSOR).
float dist_front();

// sum(weight_i * value_i) over every sensor with role == ROLE_STEER.
// Feed this straight into your PID controller as the error term.
float sensors_steer_error();
