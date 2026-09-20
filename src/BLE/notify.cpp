#include "notify.h"

#include <Arduino.h>
#include <BLECharacteristic.h>
#include <stdarg.h>

extern BLECharacteristic *characteristic; // defined in RobotBLE.cpp

void notify(const char* fmt, ...) {
  char buffer[128];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  Serial.print(buffer);

  if (!characteristic) return;
  characteristic->setValue(buffer);
  characteristic->notify();
}
