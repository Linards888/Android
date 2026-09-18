#include "sharp_logic.h"

#if Is_Sharp

// One SharpSensor object per entry in config.h's SHARP_SENSORS[] list.
const uint8_t SHARP_SENSOR_COUNT = sizeof(SHARP_SENSORS) / sizeof(SHARP_SENSORS[0]);
SharpSensor allSharpSensors[SHARP_SENSOR_COUNT];

void sharp_setup() {
  for (uint8_t i = 0; i < SHARP_SENSOR_COUNT; i++) {
    allSharpSensors[i].name          = SHARP_SENSORS[i].name;
    allSharpSensors[i].pin           = SHARP_SENSORS[i].pin;
    allSharpSensors[i].angle         = SHARP_SENSORS[i].angle;
    allSharpSensors[i].lastReadingMM = 0;
    pinMode(allSharpSensors[i].pin, INPUT);
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
    if (strcmp(allSharpSensors[i].name, name) == 0) {
      return &allSharpSensors[i];
    }
  }
  return nullptr;
}

#endif
