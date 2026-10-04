#include "notify.h"

#include <Arduino.h>
#include <stdarg.h>

#include "../../config.h"

#if Is_blueTooth
#include <BLECharacteristic.h>
extern BLECharacteristic *characteristic; // defined in RobotBLE.cpp
#endif

void notify(const char* fmt, ...) {
  char buffer[128];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  Serial.print(buffer);

#if Is_blueTooth
  if (!characteristic) return;
  characteristic->setValue(buffer);
  characteristic->notify();
#endif
}
