#include "Commands.h"

#include <Arduino.h>
#include <Preferences.h>
#include <string.h>

#include "ParamRegistry.h"
#include "RobotState.h"
#include "notify.h"

extern Preferences pref; // defined in Android.ino

namespace Commands {

namespace {
  void printParam(const char* name, const char* value) {
    notify("%s=%s\n", name, value);
  }

  void cmdGet(char* args) {
    char* name = strtok_r(args, " \r\n", &args);
    if (name == nullptr) {
      ParamRegistry::forEach(printParam);
      return;
    }

    char value[32];
    if (ParamRegistry::format(name, value, sizeof(value))) {
      notify("%s=%s\n", name, value);
    } else {
      notify("unknown param: %s\n", name);
    }
  }

  void cmdSet(char* args) {
    char* name = strtok_r(args, " \r\n", &args);
    char* value = strtok_r(args, " \r\n", &args);
    if (name == nullptr || value == nullptr) {
      notify("usage: set <name> <value>\n");
      return;
    }

    if (!ParamRegistry::set(name, value)) {
      notify("unknown param: %s\n", name);
      return;
    }

    char formatted[32];
    ParamRegistry::format(name, formatted, sizeof(formatted));
    notify("%s=%s\n", name, formatted);
  }

  void cmdSave(char* /*args*/) {
    ParamRegistry::save(pref);
    notify("saved\n");
  }

  void cmdLoad(char* /*args*/) {
    ParamRegistry::load(pref);
    notify("loaded\n");
  }

  const char* stateName(RobotState s) {
    switch (s) {
      case IDLE:        return "IDLE";
      case READY:       return "READY";
      case CALIBRATION: return "CALIBRATION";
      case COUNTDOWN:   return "COUNTDOWN";
      case RUNNING:     return "RUNNING";
      case FORWARD:     return "FORWARD";
      case BACKWARDS:   return "BACKWARDS";
    }
    return "UNKNOWN";
  }

  void cmdState(char* /*args*/) {
    notify("state=%s\n", stateName(currentState));
  }

  void cmdStart(char* /*args*/) {
    countdownStartTime = millis();
    currentState = COUNTDOWN;
    notify("state=COUNTDOWN\n");
  }

  void cmdStop(char* /*args*/) {
    currentState = IDLE;
    notify("state=IDLE\n");
  }

  void cmdCalib(char* /*args*/) {
    currentState = CALIBRATION;
    notify("state=CALIBRATION\n");
  }

  void cmdHelp(char* /*args*/) {
    notify("commands: get [name] | set <name> <value> | save | load | state | start | stop | calib | help\n");
    notify("\"get\" with no name lists every parameter\n");
  }

  struct CommandEntry {
    const char* name;
    void (*handler)(char* args);
  };

  const CommandEntry kCommands[] = {
    {"get",   cmdGet},
    {"set",   cmdSet},
    {"save",  cmdSave},
    {"load",  cmdLoad},
    {"state", cmdState},
    {"start", cmdStart},
    {"stop",  cmdStop},
    {"calib", cmdCalib},
    {"help",  cmdHelp},
  };
  const size_t kCommandCount = sizeof(kCommands) / sizeof(kCommands[0]);
}

void handle(char* command, char* args) {
  if (command == nullptr) {
    notify("unknown command\n");
    return;
  }

  for (size_t i = 0; i < kCommandCount; i++) {
    if (strcmp(command, kCommands[i].name) == 0) {
      kCommands[i].handler(args);
      return;
    }
  }
  notify("unknown command: %s\n", command);
}

void pollSerial() {
  static char buffer[64];
  static size_t length = 0;

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (length == 0) continue;
      buffer[length] = '\0';
      length = 0;

      char* rest = nullptr;
      char* command = strtok_r(buffer, " \r\n", &rest);
      handle(command, rest);
    } else if (length < sizeof(buffer) - 1) {
      buffer[length++] = c;
    }
  }
}

} // namespace Commands
