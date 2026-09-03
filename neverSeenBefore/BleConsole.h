#pragma once

#include "RuntimeConfig.h"

#if Is_blueTooth
#include <Arduino.h>
void beginBleConsole(void (*commandHandler)(String));
#endif
