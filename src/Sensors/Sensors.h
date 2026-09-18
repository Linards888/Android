#pragma once
#include <Arduino.h>
#include "config.h"

#if Is_TOF
  #include "tof_logic.h"
#endif
#if Is_Sharp
  #include "sharp_logic.h"
#endif
#if Is_Ultrasonic
  #include "ultrasonic_logic.h"
#endif

// Brings up whichever distance sensor families are enabled in config.h.
void sensors_setup();

// Distance in millimeters for the sensor with this name (e.g. "front",
// "left", "right" - whatever you named it in config.h's sensor lists),
// searched across ALL enabled sensor technologies. Returns -1 if no sensor
// with that name exists in any enabled list.
int16_t sensor_read(const char* name);
