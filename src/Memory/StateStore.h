#pragma once
#include <Arduino.h>
#include "config.h"

// ============================================================
//  StateStore — flash persistence for every ParamRegistry entry.
// ============================================================
//  Only compiled in when FEATURE_MEMORY is 1. This does NOT know
//  or care what any individual parameter means — it just walks
//  ParamRegistry (src/Utils/ParamRegistry.h) and saves/restores
//  every entry by name. That's why registering a new tunable with
//  REGISTER_PARAM automatically makes it persistent too, with no
//  changes needed here.
// ============================================================

#if FEATURE_MEMORY

void state_store_init();   // call once in setup(), AFTER registerParams()
void state_store_load();   // restores every registered param that has a saved value
void state_store_save();   // saves every registered param's current value

#endif
