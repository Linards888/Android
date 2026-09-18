#include "tof_logic.h"

#if Is_TOF

  // One TofSensor object per entry in config.h's TOF_SENSORS[] list.
  const uint8_t TOF_SENSOR_COUNT = sizeof(TOF_SENSORS) / sizeof(TOF_SENSORS[0]);
  TofSensor allTofSensors[TOF_SENSOR_COUNT];

  void tof_setup() {
    Wire.begin();

    // Step 0: copy each entry's config (name/pin/address/angle) in from
    // config.h's TOF_SENSORS[] list.
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
      allTofSensors[i].name          = TOF_SENSORS[i].name;
      allTofSensors[i].xshutPin      = TOF_SENSORS[i].xshutPin;
      allTofSensors[i].address       = TOF_SENSORS[i].i2cAddress;
      allTofSensors[i].angle         = TOF_SENSORS[i].angle;
      allTofSensors[i].lastReadingMM = 0;
    }

    // Step 1: hold every sensor's XSHUT low (disabled), so only one at a
    // time responds at the factory-default I2C address while we bring it up.
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
      pinMode(allTofSensors[i].xshutPin, OUTPUT);
      digitalWrite(allTofSensors[i].xshutPin, LOW);
    }
    delay(10);

    // Step 2: wake sensors one at a time. Each one is init()'d while still at
    // its default address (0x29), THEN moved to its configured address -
    // doing it in the other order fails because the previous sensor may
    // already be sitting on the address we're about to hand out.
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
      digitalWrite(allTofSensors[i].xshutPin, HIGH);
      delay(10);

      allTofSensors[i].sensor.setTimeout(500);
      if (!allTofSensors[i].sensor.init()) {
        Serial.print("TOF init failed: ");
        Serial.println(allTofSensors[i].name);
        continue;
      }
      allTofSensors[i].sensor.setAddress(allTofSensors[i].address);
    }
  }

  void tof_readAll() {
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
      tof_read(&allTofSensors[i]);
      Serial.print(allTofSensors[i].name);
      Serial.print(" ("); Serial.print(allTofSensors[i].angle); Serial.print("deg): ");
      Serial.println(allTofSensors[i].lastReadingMM);
    }
  }

  // Reads ONE sensor, updates its cached value, returns the reading.
  uint16_t tof_read(TofSensor* s) {
    if (s == nullptr) return 0xFFFF;   // invalid pointer, return "out of range" style value

    s->lastReadingMM = s->sensor.readRangeSingleMillimeters();

    if (s->sensor.timeoutOccurred()) {
      Serial.print("TOF timeout: ");
      Serial.println(s->name);
      return 0xFFFF; // don't let a glitched reading look like "very close"
    }

    return s->lastReadingMM;
  }

  // Convenience overload: read by name instead of pointer
  uint16_t tof_read(const char* name) {
    return tof_read(tof_getByName(name));
  }

  TofSensor* tof_getByName(const char* name) {
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
      if (strcmp(allTofSensors[i].name, name) == 0) {
        return &allTofSensors[i];
      }
    }
    return nullptr;
  }

#endif


/* ---- Tof Usage ----

//By name
uint16_t d = tof_read("front");

//by pointer
uint16_t d = tof_read(tof_getByName("front"));

*/
