#pragma once
#include <Arduino.h>

// ============================================================
//  ParamRegistry — the "add a BLE-tunable parameter in one line"
//  mechanism.
// ============================================================
//  Register any numeric/bool field of `state` (or anywhere else)
//  once, in setup(), like:
//
//      REGISTER_PARAM(state.pid.kp);
//      REGISTER_PARAM(state.speed_forward);
//
//  and it is immediately:
//    - readable/settable over BLE:      "get kp"   /  "set kp 1.5"
//    - listed by the `list` command
//    - saved to flash & restored on boot, if FEATURE_MEMORY is on
//      (see src/Memory/StateStore.cpp — it just iterates this list)
//
//  You never need to touch src/BLE/commands.cpp or src/Memory to
//  add a new tunable. Add the field to RobotState.h (or use your
//  own global), REGISTER_PARAM it in Android.ino's registerParams()
//  function, done.
//
//  REGISTER_PARAM(var) uses the variable's own name (after the
//  last '.') as the BLE-visible name. Use REGISTER_PARAM_NAMED if
//  you want a different/shorter name on the wire.
// ============================================================

enum class ParamType : uint8_t { FLOAT, U8, U16, I16, U32, BOOL };

struct ParamEntry {
    const char* name;
    ParamType type;
    void* ptr;
};

class ParamRegistry {
public:
    static constexpr int MAX_PARAMS = 64;

    static void reg(const char* name, float& v);
    static void reg(const char* name, uint8_t& v);
    static void reg(const char* name, uint16_t& v);
    static void reg(const char* name, int16_t& v);
    static void reg(const char* name, uint32_t& v);
    static void reg(const char* name, bool& v);

    static int count();
    static const ParamEntry* at(int index);
    static const ParamEntry* find(const char* name);

    // Writes a human-readable value into out (out_len bytes). Returns false if not found.
    static bool get_as_string(const char* name, char* out, size_t out_len);

    // Parses value_str according to the param's type and stores it. Returns false if not found.
    static bool set_from_string(const char* name, const char* value_str);

private:
    static void add_(const char* name, ParamType type, void* ptr);

    static ParamEntry entries_[MAX_PARAMS];
    static int count_;
};

// Extracts a short name from "state.pid.kp" -> "kp" at compile time is not
// possible in plain C++ without macros/constexpr tricks, so REGISTER_PARAM
// takes the variable and a string literal derived via the stringify + a
// small runtime helper that strips everything up to the last '.'.
const char* param_short_name(const char* full_expr);

#define REGISTER_PARAM(var) ParamRegistry::reg(param_short_name(#var), var)
#define REGISTER_PARAM_NAMED(name, var) ParamRegistry::reg(name, var)
