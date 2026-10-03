#include "ParamRegistry.h"

#include <stdlib.h>
#include <string.h>

#include "Params.h"

namespace ParamRegistry {

namespace {
  enum class Type : uint8_t { FLOAT, INT, UINT16, BOOL };

  struct Desc {
    const char* name;    // used in "get"/"set" commands over BLE/Serial
    const char* nvsKey;  // <=15 chars - ESP32 Preferences/NVS key limit
    Type type;
    void* ptr;
  };

  const Desc kParams[] = {
    // ---- PID ----
    {"kp",             "kp",             Type::FLOAT, &kp},
    {"ki",             "ki",             Type::FLOAT, &ki},
    {"kd",             "kd",             Type::FLOAT, &kd},
    {"kleft",          "kleft",          Type::FLOAT, &kleft},
    {"kright",         "kright",         Type::FLOAT, &kright},
    {"maxdelta",       "maxdelta",       Type::INT,   &maxdelta},
    {"constrainpid",   "constrainpid",   Type::INT,   &constrainpid},
    {"constraindelta", "constraindelta", Type::INT,   &constraindelta},

    // ---- Drive speeds ----
    {"maxspeed", "maxspeed", Type::INT, &maxspeed},
    {"minspeed", "minspeed", Type::INT, &minspeed},
    {"fspeed",   "fspeed",   Type::INT, &fspeed},
    {"rspeed",   "rspeed",   Type::INT, &rspeed},

    // ---- Sensor thresholds ----
    {"errorleftdist",    "errorleftdist",  Type::INT, &errorleftdist},
    {"errorrightdist",   "errorrightdist", Type::INT, &errorrightdist},
    {"minwalldistfront", "minwallfront",   Type::INT, &minwalldistFront},

    // ---- Motor / actuator tuning ----
    {"drive_max",           "drive_max",      Type::INT, &DRIVE_MAX},
    {"pwm_freq_hz",          "pwm_freq_hz",    Type::INT, &PWM_FREQ_HZ},
    {"pwm_res_bits",         "pwm_res_bits",   Type::INT, &PWM_RESOLUTION_BITS},
    {"esc_min_us",           "esc_min_us",     Type::INT, &ESC_MIN_US},
    {"esc_neutral_us",       "esc_neutral_us", Type::INT, &ESC_NEUTRAL_US},
    {"esc_max_us",           "esc_max_us",     Type::INT, &ESC_MAX_US},
    {"steer_min_deg",        "steer_min_deg",  Type::INT, &STEER_MIN_DEG},
    {"steer_center_deg",     "steer_ctr_deg",  Type::INT, &STEER_CENTER_DEG},
    {"steer_max_deg",        "steer_max_deg",  Type::INT, &STEER_MAX_DEG},
    {"servo_pulse_min_us",   "servo_min_us",   Type::INT, &SERVO_PULSE_MIN_US},
    {"servo_pulse_max_us",   "servo_max_us",   Type::INT, &SERVO_PULSE_MAX_US},

    // ---- Debug ----
    {"dbg_log_distance",     "dbg_dist",     Type::BOOL,   &debugState.logDistance},
    {"dbg_log_distance_ms",  "dbg_dist_ms",  Type::UINT16, &debugState.logDistanceIntervalMs},
    {"dbg_log_sensors",      "dbg_sens",     Type::BOOL,   &debugState.logSensors},
    {"dbg_log_sensors_ms",   "dbg_sens_ms",  Type::UINT16, &debugState.logSensorsIntervalMs},
    {"dbg_log_pid",          "dbg_pid",      Type::BOOL,   &debugState.logPid},
    {"dbg_log_pid_ms",       "dbg_pid_ms",   Type::UINT16, &debugState.logPidIntervalMs},
    {"dbg_ble_telemetry",    "dbg_tele",     Type::BOOL,   &debugState.bleTelemetry},
    {"dbg_ble_telemetry_ms", "dbg_tele_ms",  Type::UINT16, &debugState.bleTelemetryIntervalMs},
  };
  const size_t kParamCount = sizeof(kParams) / sizeof(kParams[0]);

  const Desc* find(const char* name) {
    for (size_t i = 0; i < kParamCount; i++) {
      if (strcmp(kParams[i].name, name) == 0) return &kParams[i];
    }
    return nullptr;
  }
}

bool format(const char* name, char* out, size_t outSize) {
  const Desc* p = find(name);
  if (!p) return false;

  switch (p->type) {
    case Type::FLOAT:  snprintf(out, outSize, "%.4f", *(float*)p->ptr); break;
    case Type::INT:    snprintf(out, outSize, "%d", *(int*)p->ptr); break;
    case Type::UINT16: snprintf(out, outSize, "%u", *(uint16_t*)p->ptr); break;
    case Type::BOOL:   snprintf(out, outSize, "%d", *(bool*)p->ptr ? 1 : 0); break;
  }
  return true;
}

bool set(const char* name, const char* value) {
  const Desc* p = find(name);
  if (!p) return false;

  switch (p->type) {
    case Type::FLOAT:  *(float*)p->ptr = (float)atof(value); break;
    case Type::INT:    *(int*)p->ptr = atoi(value); break;
    case Type::UINT16: *(uint16_t*)p->ptr = (uint16_t)atoi(value); break;
    case Type::BOOL:   *(bool*)p->ptr = atoi(value) != 0; break;
  }
  return true;
}

void forEach(void (*fn)(const char* name, const char* value)) {
  char value[32];
  for (size_t i = 0; i < kParamCount; i++) {
    format(kParams[i].name, value, sizeof(value));
    fn(kParams[i].name, value);
  }
}

void load(Preferences& prefs) {
  for (size_t i = 0; i < kParamCount; i++) {
    const Desc& p = kParams[i];
    if (!prefs.isKey(p.nvsKey)) continue;

    switch (p.type) {
      case Type::FLOAT:  *(float*)p.ptr = prefs.getFloat(p.nvsKey); break;
      case Type::INT:    *(int*)p.ptr = prefs.getInt(p.nvsKey); break;
      case Type::UINT16: *(uint16_t*)p.ptr = prefs.getUShort(p.nvsKey); break;
      case Type::BOOL:   *(bool*)p.ptr = prefs.getBool(p.nvsKey); break;
    }
  }
}

void save(Preferences& prefs) {
  for (size_t i = 0; i < kParamCount; i++) {
    const Desc& p = kParams[i];
    switch (p.type) {
      case Type::FLOAT:  prefs.putFloat(p.nvsKey, *(float*)p.ptr); break;
      case Type::INT:    prefs.putInt(p.nvsKey, *(int*)p.ptr); break;
      case Type::UINT16: prefs.putUShort(p.nvsKey, *(uint16_t*)p.ptr); break;
      case Type::BOOL:   prefs.putBool(p.nvsKey, *(bool*)p.ptr); break;
    }
  }
}

} // namespace ParamRegistry
