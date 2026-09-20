#include "tof_logic.h"

namespace Tof {

namespace {
  // Every sensor this build knows about, in the same order they're
  // defined in config.h. read() matches the TOFSensor reference it's
  // given against this list by address, so callers just pass one of
  // the globals, e.g. Tof::read(Front).
  const TOFSensor *configs[] = { &Front, &Right, &Left, &RightSide, &LeftSide };
  const size_t sensorCount = sizeof(configs) / sizeof(configs[0]);

  VL53L0X devices[sensorCount];
  bool ready[sensorCount] = { false };
}

void setupAll() {   // (still named TofInit right now - see the mismatch I flagged earlier)
  Wire.begin();

  for (size_t i = 0; i < sensorCount; i++) {
    if (configs[i]->pin == -1) {
      ready[i] = false;
      continue; // not installed - never touch this pin
    }
    pinMode(configs[i]->pin, OUTPUT);
    digitalWrite(configs[i]->pin, LOW);
  }
  delay(10);

  for (size_t i = 0; i < sensorCount; i++) {
    if (configs[i]->pin == -1) {
      continue; // already marked not-ready above
    }
    digitalWrite(configs[i]->pin, HIGH);
    delay(10);

    devices[i].setTimeout(500);
    if (devices[i].init()) {
      devices[i].setAddress(configs[i]->address);
      devices[i].startContinuous();
      ready[i] = true;
    } else {
      ready[i] = false;
      Serial.print("Tof: ");
      Serial.print(names[i]);
      Serial.println(" sensor failed to init");
    }
  }
}

uint16_t read(const TOFSensor &sensor) {
  for (size_t i = 0; i < sensorCount; i++) {
    if (configs[i] == &sensor) {
      if (!ready[i]) {
        return -1;
      }

      uint16_t distance = devices[i].readRangeContinuousMillimeters();
      if (devices[i].timeoutOccurred()) {
        return -1;
      }
      return distance;
    }
  }

  return -1; // not one of the sensors configured in config.h
}

} // namespace Tof