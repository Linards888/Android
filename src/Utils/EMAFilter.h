#pragma once

// Simple exponential moving-average filter, used to smooth noisy sensor
// readings (Sharp IR in particular is jumpy). alpha is 0..1: higher = less
// smoothing / more responsive, lower = smoother / laggier.
struct EMAFilter {
    float alpha;
    float filtered_value = 0.0f;
    bool primed = false;

    explicit EMAFilter(float alpha_) : alpha(alpha_) {}

    float update(float current_value) {
        if (!primed) {
            filtered_value = current_value;
            primed = true;
        } else {
            filtered_value = (current_value * alpha) + (filtered_value * (1.0f - alpha));
        }
        return filtered_value;
    }

    float get() const { return filtered_value; }
};
