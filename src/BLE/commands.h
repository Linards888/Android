#pragma once

// ============================================================
//  commands.h — the BLE command table.
// ============================================================
//  Adding a brand-new ACTION command (one that does something,
//  rather than just reading/writing a number) is 3 lines:
//
//      static void cmd_wave(char* args) {
//          notify("waving!\n");
//          // ... whatever it should do ...
//      }
//
//  and one line in command_list[] in commands.cpp:
//
//      {"wave", cmd_wave},
//
//  If you just want a new TUNABLE NUMBER exposed over BLE, you
//  don't need a command at all — see src/Utils/ParamRegistry.h
//  and REGISTER_PARAM in Android.ino's registerParams(). The
//  built-in `get`/`set`/`list`/`toggle` commands below already
//  cover every registered parameter automatically.
// ============================================================

struct CommandEntry {
    const char* name;
    void (*handler)(char* args);
};

// Parses "command args..." (already split by RobotBLE.cpp) and runs it.
void handle_command(char* command, char* args);
