#pragma once
#include "config.h"

#if Memory
  // Reads any previously-saved parameters from flash (ESP32 NVS) into
  // `state`. Fields with nothing saved yet keep the config.h DEFAULT_* value
  // `state` was already constructed with. Call once, in setup(), before
  // anything else reads `state`.
  void load_state();

  // Writes the current tunable fields of `state` to flash. Called from
  // loop() when state.save_requested is set (by the BLE "save" command) -
  // never called directly from a BLE callback, since flash writes can take
  // a few ms and shouldn't run inside the BLE stack's own task.
  void save_state();
#endif
