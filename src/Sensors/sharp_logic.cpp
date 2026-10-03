#include "sharp_logic.h"
#include <math.h>

namespace Sharp {

namespace {
  // Every sensor this build knows about, in the same order they're
  // defined in config.h. read() matches the SharpSensor reference it's
  // given against this list by address, so callers just pass one of
  // the globals, e.g. Sharp::read(SharpFront).
  const SharpSensor *configs[] = { &SharpFront, &SharpRight, &SharpLeft, &SharpRightSide, &SharpLeftSide };
  const size_t sensorCount = sizeof(configs) / sizeof(configs[0]);

  // Sharp analog IR sensors (e.g. GP2Y0A21YK0F) don't put out a distance
  // directly - every unit follows roughly the same curve, but the exact
  // fit varies sensor to sensor. Re-tune these against docs/Calibration.md
  // once you have real readings, or replace readCm() with your sensor's
  // datasheet formula if you're using a different model.
  const float ADC_MAX = 4095.0;  // ESP32 ADC is 12-bit
  const float ADC_VREF = 3.3;    // ESP32 ADC reference voltage, in volts

  // Converts one raw analog reading into a distance in centimeters.
  // Returns -1 if the reading isn't usable (e.g. sensor unplugged).
  float readCm(uint8_t pin) {
    int raw = analogRead(pin);
    float voltage = raw * (ADC_VREF / ADC_MAX);
    if (voltage <= 0.0) {
      return -1;
    }

    // Generic GP2Y0A21YK0F-style curve, valid roughly over its 10-80cm
    // range. Swap this out for whatever your specific sensor needs.
    return 27.86 * pow(voltage, -1.15);
  }
}

void setupAll() {
  for (size_t i = 0; i < sensorCount; i++) {
    if (configs[i]->pin == -1) {
      continue; // not installed - leave the pin alone
    }
    pinMode(configs[i]->pin, INPUT);
  }
}

uint16_t read(const SharpSensor &sensor) {
  for (size_t i = 0; i < sensorCount; i++) {
    if (configs[i] == &sensor) {
      if (configs[i]->pin == -1) {
        return -1; // not installed
      }
      float cm = readCm(configs[i]->pin);
      if (cm <= 0.0) {
        return -1;
      }
      return (uint16_t)(cm * 10.0);
    }
  }
  return -1;
}

} // namespace Sharp
