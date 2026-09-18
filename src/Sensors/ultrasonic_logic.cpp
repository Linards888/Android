#include "ultrasonic_logic.h"

#if Is_Ultrasonic

// One UltrasonicSensor object per entry in config.h's USONIC_SENSORS[] list.
const uint8_t USONIC_SENSOR_COUNT = sizeof(USONIC_SENSORS) / sizeof(USONIC_SENSORS[0]);
UltrasonicSensor allUsonicSensors[USONIC_SENSOR_COUNT];

void ultrasonic_setup() {
  for (uint8_t i = 0; i < USONIC_SENSOR_COUNT; i++) {
    allUsonicSensors[i].name          = USONIC_SENSORS[i].name;
    allUsonicSensors[i].trigPin       = USONIC_SENSORS[i].trigPin;
    allUsonicSensors[i].echoPin       = USONIC_SENSORS[i].echoPin;
    allUsonicSensors[i].angle         = USONIC_SENSORS[i].angle;
    allUsonicSensors[i].lastReadingMM = 0;

    pinMode(allUsonicSensors[i].trigPin, OUTPUT);
    digitalWrite(allUsonicSensors[i].trigPin, LOW);
    pinMode(allUsonicSensors[i].echoPin, INPUT);
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
    if (strcmp(allUsonicSensors[i].name, name) == 0) {
      return &allUsonicSensors[i];
    }
  }
  return nullptr;
}

#endif
