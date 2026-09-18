#pragma once
#include <Arduino.h>

// Tracks how much time has passed between successive update() calls.
struct DeltaTime {
	unsigned long currentTime = 0;
	unsigned long oldTime = 0;
	float dt = 0; // milliseconds

	void update() {
		currentTime = millis();
		dt = currentTime - oldTime;
		oldTime = currentTime;
	}

	float get() {
		return dt; // milliseconds
	}

	float getSeconds() {
		return dt / 1000.0f;
	}

	void set_prev_time(unsigned long time) {
		oldTime = time;
	}
};
