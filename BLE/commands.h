#pragma once

struct CommandEntry {
    const char* name;
    void(*handler)(char* args);
};

void handle_command(char* command, char* args);
