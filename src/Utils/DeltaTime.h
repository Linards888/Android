#pragma once
#include <Arduino.h>

struct DeltaTime {
	unsigned long currentTime;
	unsigned long oldTime;
	float dt;

	void update() {
		currentTime = millis();
		dt = currentTime - oldTime;
		oldTime = currentTime;
	}

	float get() {
		return dt;
	}

	void set_prev_time(unsigned long time) {
		oldTime = time;
	}
};
		
