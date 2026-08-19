#pragma once

struct CommandEntry {
    const char* name;
    void(*handler)(char* args);
    uint8_t allowed_states;
};

void handle_command(char* command, char* args);
