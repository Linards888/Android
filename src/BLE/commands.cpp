#include "commands.h"
#include "config.h"
#include "../Utils/RobotState.h"
#include "../Utils/Notify.h"
#include "../Utils/ParamRegistry.h"
#include <string.h>
#include <stdlib.h>

// ---- action commands ----

static void cmd_ready(char* args) {
    state.running_state = RunningState::READY;
    notify("ready\n");
}

static void cmd_start(char* args) {
    state.start_time_ms = millis();
    state.running_state = RunningState::COUNTDOWN;
    notify("starting in %u ms\n", state.start_delay_ms);
}

static void cmd_stop(char* args) {
    state.running_state = RunningState::IDLE;
    notify("stopped\n");
}

static void cmd_calibrate(char* args) {
    state.running_state = RunningState::CALIBRATING;
}

static void cmd_fwd(char* args) {
    state.running_state = RunningState::MANUAL_FORWARD;
    notify("manual forward\n");
}

static void cmd_bwd(char* args) {
    state.running_state = RunningState::MANUAL_BACKWARD;
    notify("manual backward\n");
}

static void cmd_180(char* args) {
    state.debug.do_manual_180 = true;
}

static void cmd_save(char* args) {
#if FEATURE_MEMORY
    state.save_state = true;
#else
    notify("FEATURE_MEMORY is off in your config — nothing to save\n");
#endif
}

// Snaps every quick-tunable back to the compiled-in defaults from
// src/Core/RobotLogic.h's `tunable` namespace — a "factory reset" without
// reflashing. Does NOT touch flash by itself; `save` afterwards if you want
// the reset to survive a reboot too.
static void cmd_defaults(char* args) {
    state.reset_defaults = true;
}

static void cmd_state(char* args) {
    notify("state=%s kp=%.3f ki=%.3f kd=%.3f fwd=%u rev=%u\n",
           running_state_name(state.running_state),
           state.pid.kp, state.pid.ki, state.pid.kd,
           state.speed_forward, state.speed_reverse);
}

// ---- generic parameter access (this is what makes adding a new
//      *tunable* free — see src/Utils/ParamRegistry.h) ----

static void cmd_get(char* args) {
    char* name = strtok_r(args, " \r\n", &args);
    if (name == nullptr) { notify("usage: get <name>\n"); return; }

    char value[32];
    if (ParamRegistry::get_as_string(name, value, sizeof(value))) {
        notify("%s %s\n", name, value);
    } else {
        notify("unknown param '%s' (try 'list')\n", name);
    }
}

static void cmd_set(char* args) {
    char* name = strtok_r(args, " \r\n", &args);
    char* value_str = strtok_r(args, " \r\n", &args);
    if (name == nullptr || value_str == nullptr) { notify("usage: set <name> <value>\n"); return; }

    if (ParamRegistry::set_from_string(name, value_str)) {
        char value[32];
        ParamRegistry::get_as_string(name, value, sizeof(value));
        notify("%s %s\n", name, value);
    } else {
        notify("unknown param '%s' (try 'list')\n", name);
    }
}

static void cmd_toggle(char* args) {
    char* name = strtok_r(args, " \r\n", &args);
    if (name == nullptr) { notify("usage: toggle <name>\n"); return; }

    const ParamEntry* e = ParamRegistry::find(name);
    if (e == nullptr || e->type != ParamType::BOOL) {
        notify("'%s' isn't a toggleable on/off param (try 'list')\n", name);
        return;
    }
    bool* value = static_cast<bool*>(e->ptr);
    *value = !*value;
    notify("%s %d\n", name, *value ? 1 : 0);
}

static void cmd_list(char* args) {
    notify("%d parameter(s):\n", ParamRegistry::count());
    char value[32];
    for (int i = 0; i < ParamRegistry::count(); i++) {
        const ParamEntry* e = ParamRegistry::at(i);
        ParamRegistry::get_as_string(e->name, value, sizeof(value));
        notify("  %s %s\n", e->name, value);
    }
}

static void cmd_help(char* args);

// ---- the command table ----
// Add a new action command here — one line, `{"name", handler}`.

static const CommandEntry command_list[] = {
    {"ready",     cmd_ready},
    {"start",     cmd_start},
    {"stop",      cmd_stop},
    {"calibrate", cmd_calibrate},
    {"fwd",       cmd_fwd},
    {"bwd",       cmd_bwd},
    {"180",       cmd_180},
    {"save",      cmd_save},
    {"defaults",  cmd_defaults},
    {"state",     cmd_state},

    {"get",       cmd_get},
    {"set",       cmd_set},
    {"toggle",    cmd_toggle},
    {"list",      cmd_list},

    {"help",      cmd_help},
};
static const int COMMAND_COUNT = sizeof(command_list) / sizeof(command_list[0]);

static void cmd_help(char* args) {
    notify("commands:\n");
    for (int i = 0; i < COMMAND_COUNT; i++) {
        notify("  %s\n", command_list[i].name);
    }
    notify("get/set/toggle work on any parameter from 'list'\n");
}

void handle_command(char* command, char* args) {
    for (int i = 0; i < COMMAND_COUNT; i++) {
        if (strcmp(command, command_list[i].name) == 0) {
            command_list[i].handler(args);
            return;
        }
    }
    notify("unknown command '%s' (try 'help')\n", command);
}
