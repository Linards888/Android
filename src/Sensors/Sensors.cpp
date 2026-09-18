#include "Sensors.h"

void sensors_setup() {
  #if Is_TOF
    tof_setup();
  #endif
  #if Is_Sharp
    sharp_setup();
  #endif
  #if Is_Ultrasonic
    ultrasonic_setup();
  #endif
}

int16_t sensor_read(const char* name) {
  #if Is_TOF
    if (TofSensor* s = tof_getByName(name)) {
      uint16_t mm = tof_read(s);
      return mm == 0xFFFF ? -1 : (int16_t)mm;
    }
  #endif
  #if Is_Sharp
    if (SharpSensor* s = sharp_getByName(name)) {
      uint16_t mm = sharp_read(s);
      return mm == 0xFFFF ? -1 : (int16_t)mm;
    }
  #endif
  #if Is_Ultrasonic
    if (UltrasonicSensor* s = ultrasonic_getByName(name)) {
      uint16_t mm = ultrasonic_read(s);
      return mm == 0xFFFF ? -1 : (int16_t)mm;
    }
  #endif
  return -1;
}
