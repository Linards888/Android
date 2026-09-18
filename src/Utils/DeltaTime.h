#pragma once
#include <Arduino.h>

// Tracks the time (in seconds) elapsed between successive update() calls.
// Used to keep PID math independent of how long a loop iteration actually took.
struct DeltaTime {
    unsigned long current_time = 0;
    unsigned long previous_time = 0;
    float dt = 0.0f;

    void update() {
        current_time = millis();
        dt = (current_time - previous_time) / 1000.0f; // seconds
        previous_time = current_time;
    }

    float get() const {
        return dt;
    }

    void set_prev_time(unsigned long time) {
        previous_time = time;
    }
};
