#pragma once
#include <Arduino.h>
#include "config.h"

#if Is_blueTooth

struct CommandEntry {
    const char* name;
    void(*handler)(char* args);
    uint8_t allowed_states;
};

// Splits nothing itself - `command` is the first whitespace-separated token
// already pulled out of the BLE payload, `args` is whatever's left (may be
// an empty string, never NULL). Looks `command` up in the table and, if the
// current mode is allowed to run it, calls its handler.
void handle_command(char* command, char* args);

#endif
