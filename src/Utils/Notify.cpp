#include "Notify.h"
#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>

static NotifySink extra_sink = nullptr;

void notify_set_sink(NotifySink sink) {
    extra_sink = sink;
}

void notify(const char* fmt, ...) {
    char buffer[160];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    Serial.print(buffer);

    if (extra_sink) {
        extra_sink(buffer);
    }
}
