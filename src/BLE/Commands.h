#pragma once

// Shared text-command protocol used by both BLE (RobotBLE.cpp) and,
// optionally, the Serial console - so the same commands work over
// either transport, and both end up going through the same parameter
// registry (ParamRegistry.h). Replies are sent through notify()
// (notify.h).
//
//   get                    -> dump every parameter as name=value
//   get <name>             -> read one parameter
//   set <name> <value>     -> write one parameter (in RAM only)
//   save                   -> persist every parameter to flash
//   load                   -> reload every parameter from flash
//   state                  -> report the current RobotState
//   start / stop / calib   -> change RobotState
//   help                   -> list commands
namespace Commands {
  // Parses and runs one line: `command` is the first whitespace-
  // separated token, `args` is everything after it (may be nullptr) -
  // the same split strtok_r(line, " ", &rest) gives you.
  void handle(char* command, char* args);

  // Reads any bytes currently waiting on Serial and calls handle() once
  // a full line has come in. Call this once per loop() iteration to be
  // able to tune parameters from the Serial monitor too, not just BLE.
  void pollSerial();
}
