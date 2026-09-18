#include "ultrasonic_logic.h"

#if Is_Ultrasonic

#define X(name, trigPin, echoPin, angle) UltrasonicSensor usonic_##name = { #name, trigPin, echoPin, angle, 0 };
  USONIC_SENSOR_LIST
#undef X

#define X(name, trigPin, echoPin, angle) &usonic_##name,
  UltrasonicSensor* allUsonicSensors[] = { USONIC_SENSOR_LIST };
#undef X
const uint8_t USONIC_SENSOR_COUNT = sizeof(allUsonicSensors) / sizeof(allUsonicSensors[0]);

void ultrasonic_setup() {
  for (uint8_t i = 0; i < USONIC_SENSOR_COUNT; i++) {
    pinMode(allUsonicSensors[i]->trigPin, OUTPUT);
    digitalWrite(allUsonicSensors[i]->trigPin, LOW);
    pinMode(allUsonicSensors[i]->echoPin, INPUT);
  }
}

// Standard HC-SR04-style trigger/echo timing. pulseIn() blocks for up to
// USONIC_TIMEOUT_US (config.h) if nothing is in range, so keep that value
// reasonable - it directly adds to how long one control loop tick can take.
uint16_t ultrasonic_read(UltrasonicSensor* s) {
  if (s == nullptr) return 0xFFFF;

  digitalWrite(s->trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(s->trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(s->trigPin, LOW);

  unsigned long durationUs = pulseIn(s->echoPin, HIGH, USONIC_TIMEOUT_US);
  if (durationUs == 0) {
    // timed out - nothing detected within range
    s->lastReadingMM = 0xFFFF;
    return s->lastReadingMM;
  }

  // speed of sound ~343 m/s = 0.343 mm/us, round trip -> divide by 2
  s->lastReadingMM = (uint16_t)((float)durationUs * 0.343f / 2.0f);
  return s->lastReadingMM;
}

uint16_t ultrasonic_read(const char* name) {
  return ultrasonic_read(ultrasonic_getByName(name));
}

UltrasonicSensor* ultrasonic_getByName(const char* name) {
  for (uint8_t i = 0; i < USONIC_SENSOR_COUNT; i++) {
    if (strcmp(allUsonicSensors[i]->name, name) == 0) {
      return allUsonicSensors[i];
    }
  }
  return nullptr;
}

#endif
