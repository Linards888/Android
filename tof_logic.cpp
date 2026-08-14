#include "tof_logic.h"

//Es nesaprotu tik daudz, vai arī ja saprotu tad aizmirsu un tagat vairs neatceros, lūdzu palīdzat man

// Generates: TofSensor tof_front = {VL53L0X(), "front", 4, 0x30, 0, 0};  etc.
  #define X(name, pin, addr, angle) TofSensor tof_##name = { VL53L0X(), #name, pin, addr, angle, 0 };
    TOF_SENSOR_LIST
  #undef X

  // Build a pointer array so setup/read loops can iterate all sensors generically
  #define X(name, pin, addr, angle) &tof_##name,
    TofSensor* allTofSensors[] = { TOF_SENSOR_LIST };
  #undef X
  const uint8_t TOF_SENSOR_COUNT = sizeof(allTofSensors) / sizeof(allTofSensors[0]);

  void tof_setup() {
    Wire.begin();

    // Step 1: hold every sensor's XSHUT low (disabled)
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
      pinMode(allTofSensors[i]->xshutPin, OUTPUT);
      digitalWrite(allTofSensors[i]->xshutPin, LOW);
    }

    // Step 2: wake sensors one at a time, assign each a unique address
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
      digitalWrite(allTofSensors[i]->xshutPin, HIGH);
      delay(10);
      allTofSensors[i]->sensor.setAddress(allTofSensors[i]->address);
      allTofSensors[i]->sensor.init();
    }
  }

  void tof_readAll() {
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
      allTofSensors[i]->lastReadingMM = allTofSensors[i]->sensor.readRangeSingleMillimeters();
      Serial.print(allTofSensors[i]->name);
      Serial.print(" ("); Serial.print(allTofSensors[i]->angle); Serial.print("deg): ");
      Serial.println(allTofSensors[i]->lastReadingMM);
    }
  }

  // Reads ONE sensor, updates its cached value, returns the reading.
  uint16_t tof_read(TofSensor* s) {
    if (s == nullptr) return 0xFFFF;   // invalid pointer, return "out of range" style value

    s->lastReadingMM = s->sensor.readRangeSingleMillimeters();

    if (s->sensor.timeoutOccurred()) {
      Serial.print("TOF timeout: ");
      Serial.println(s->name);
    }

    return s->lastReadingMM;
  }

  // Convenience overload: read by name instead of pointer
  uint16_t tof_read(const char* name) {
    return tof_read(tof_getByName(name));
  }

  TofSensor* tof_getByName(const char* name) {
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; i++) {
      if (strcmp(allTofSensors[i]->name, name) == 0) {
        return allTofSensors[i];
      }
    }
    return nullptr;
  }

#endif


/* ---- Tof Usage ----

//By name
uint16_t d = tof_read("front");

//by pointer
uint16_t d = tof_read(&tof_front);

*/