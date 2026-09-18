#include "sharp_logic.h"

#if Is_Sharp

#define X(name, pin, angle) SharpSensor sharp_##name = { #name, pin, angle, 0 };
  SHARP_SENSOR_LIST
#undef X

#define X(name, pin, angle) &sharp_##name,
  SharpSensor* allSharpSensors[] = { SHARP_SENSOR_LIST };
#undef X
const uint8_t SHARP_SENSOR_COUNT = sizeof(allSharpSensors) / sizeof(allSharpSensors[0]);

void sharp_setup() {
  for (uint8_t i = 0; i < SHARP_SENSOR_COUNT; i++) {
    pinMode(allSharpSensors[i]->pin, INPUT);
  }
}

// Sharp IR sensors aren't linear, but a straight line between two measured
// points (SHARP_ADC_AT_NEAR/FAR <-> SHARP_NEAR_MM/FAR_MM, set in config.h) is
// close enough for wall-following. See docs/Calibration.md.
uint16_t sharp_read(SharpSensor* s) {
  if (s == nullptr) return 0xFFFF;

  int raw = analogRead(s->pin);
  long mm = map(raw, SHARP_ADC_AT_NEAR, SHARP_ADC_AT_FAR, SHARP_NEAR_MM, SHARP_FAR_MM);
  mm = constrain(mm, (long)SHARP_NEAR_MM, (long)SHARP_FAR_MM);

  s->lastReadingMM = (uint16_t)mm;
  return s->lastReadingMM;
}

uint16_t sharp_read(const char* name) {
  return sharp_read(sharp_getByName(name));
}

SharpSensor* sharp_getByName(const char* name) {
  for (uint8_t i = 0; i < SHARP_SENSOR_COUNT; i++) {
    if (strcmp(allSharpSensors[i]->name, name) == 0) {
      return allSharpSensors[i];
    }
  }
  return nullptr;
}

#endif
