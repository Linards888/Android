#pragma once
#include <Arduino.h>
#include <Preferences.h>

// Generic name <-> variable lookup for every value in Params.h, used by
// Commands.cpp to implement "get" / "set" / "save" / "load" without a
// hand-written function per parameter.
//
// Adding a new tunable: declare it in Params.h, define+default it in
// Params.cpp, then add one line to the table in ParamRegistry.cpp - that
// is the only extra place that needs to know its name.
namespace ParamRegistry {
  // Writes the named parameter's current value as text into out.
  // Returns false (out left untouched) if no parameter has that name.
  bool format(const char* name, char* out, size_t outSize);

  // Parses value and assigns it to the named parameter.
  // Returns false (nothing changed) if no parameter has that name.
  bool set(const char* name, const char* value);

  // Calls fn(name, valueAsText) once per registered parameter, in table
  // order. Used for "get" with no name (dump everything).
  void forEach(void (*fn)(const char* name, const char* value));

  // Loads every registered parameter from flash (NVS, via Preferences).
  // A parameter never saved before is left at its Params.cpp default.
  void load(Preferences& prefs);

  // Persists every registered parameter to flash.
  void save(Preferences& prefs);
}
