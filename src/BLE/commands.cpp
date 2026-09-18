#include "commands.h"

#if Is_blueTooth

#include "RobotState.h"

#if Is_IMU
  #include "IMU_logic.h"
#endif

extern void notify(const char* fmt, ...);

// ---- Generic helpers ----

template<typename T>
static void set_or_report_int(char*& args, T* target, const char* fmt) {
  char* value = strtok_r(args, " \r\n", &args);
  if (value != NULL) *target = static_cast<T>(atol(value));
  notify(fmt, *target);
}

static void set_or_report_float(char*& args, float* target, const char* fmt) {
  char* value = strtok_r(args, " \r\n", &args);
  if (value != NULL) *target = atof(value);
  notify(fmt, *target);
}

static void toggle_bool(bool* target, const char* fmt) {
  *target = !*target;
  notify(fmt, *target);
}

// ---- k (PID / steering gain) params ----

struct FloatEntry { const char* name; float* target; const char* fmt; };

static FloatEntry k_list[] = {
  {"p",             &state.pid.kp,             "kp %.4f\n"},
  {"i",             &state.pid.ki,             "ki %.4f\n"},
  {"d",             &state.pid.kd,             "kd %.4f\n"},
  {"reverse",       &state.k_reverse,          "k_reverse %.4f\n"},
  {"left_side",     &state.pid.k_left_side,    "k_left_side %.4f\n"},
  {"left",          &state.pid.k_left,         "k_left %.4f\n"},
  {"right_side",    &state.pid.k_right_side,   "k_right_side %.4f\n"},
  {"right",         &state.pid.k_right,        "k_right %.4f\n"},
  {"pitch_running", &state.k_pitch_running,    "k_pitch_running %.4f\n"},
  {"accel_nudge",   &state.k_accel_nudge,      "k_accel_nudge %.4f\n"},
};

void cmd_k(char* args) {
  char* argument = strtok_r(args, " \r\n", &args);
  if (argument == NULL) {
    notify("kp %.4f ki %.4f kd %.4f k_reverse %.4f k_left %.4f k_left_side %.4f k_right %.4f k_right_side %.4f\n",
      state.pid.kp, state.pid.ki, state.pid.kd, state.k_reverse,
      state.pid.k_left, state.pid.k_left_side, state.pid.k_right, state.pid.k_right_side);
    return;
  }
  for (auto& entry : k_list) {
    if (strcmp(argument, entry.name) == 0) {
      set_or_report_float(args, entry.target, entry.fmt);
      return;
    }
  }
  notify("unknown k subcommand\n");
}

// ---- speed ----

void cmd_speed(char* args) {
  char* argument = strtok_r(args, " \r\n", &args);
  if (argument == NULL) {
    notify("scaled %d min %d max %d forward %d reverse %d\n",
      state.scaled_speed, state.speed_min, state.speed_max, state.speed_forward, state.speed_reverse);
    return;
  }
  if      (strcmp(argument, "forward") == 0) set_or_report_int(args, &state.speed_forward, "forward speed %d\n");
  else if (strcmp(argument, "reverse") == 0) set_or_report_int(args, &state.speed_reverse, "reverse speed %d\n");
  else if (strcmp(argument, "min") == 0)     set_or_report_int(args, &state.speed_min,     "speed min %d\n");
  else if (strcmp(argument, "max") == 0)     set_or_report_int(args, &state.speed_max,     "speed max %d\n");
  else notify("unknown speed subcommand\n");
}

// ---- dist ----

void cmd_dist(char* args) {
  char* argument = strtok_r(args, " \r\n", &args);
  if (argument == NULL) {
    notify("near %d far %d reverse %d constrain %d\n",
      state.dist_near, state.dist_far, state.dist_reverse, state.dist_constrain);
    return;
  }
  if      (strcmp(argument, "near") == 0)      set_or_report_int(args, &state.dist_near,      "near distance %d\n");
  else if (strcmp(argument, "far") == 0)       set_or_report_int(args, &state.dist_far,       "far distance %d\n");
  else if (strcmp(argument, "reverse") == 0)   set_or_report_int(args, &state.dist_reverse,   "reverse distance %d\n");
  else if (strcmp(argument, "constrain") == 0) set_or_report_int(args, &state.dist_constrain, "constrain %d\n");
  else notify("unknown dist subcommand\n");
}

// ---- log ----

struct LogEntry { const char* name; bool* enabled; uint16_t* interval_ms; const char* label; };

static LogEntry log_list[] = {
  {"dist",  &state.debug.log_distance, &state.debug.log_distance_interval_ms, "distance"},
#if Is_IMU
  {"gyro",  &state.debug.log_gyro,     &state.debug.log_gyro_interval_ms,     "gyro"},
  {"accel", &state.debug.log_accel,    &state.debug.log_accel_interval_ms,    "accel"},
  {"yaw",   &state.debug.log_yaw,      &state.debug.log_yaw_interval_ms,      "yaw"},
  {"pitch", &state.debug.log_pitch,    &state.debug.log_pitch_interval_ms,    "pitch"},
#endif
};

void cmd_log(char* args) {
  char* argument = strtok_r(args, " \r\n", &args);
  if (argument == NULL) { notify("usage: log <dist|gyro|accel|yaw|pitch> [interval_ms]\n"); return; }

  char* value = strtok_r(args, " \r\n", &args);
  for (auto& entry : log_list) {
    if (strcmp(argument, entry.name) == 0) {
      *entry.enabled = !*entry.enabled;
      if (value != NULL) *entry.interval_ms = atol(value);
      notify("log %s %s (%d ms)\n", entry.label, *entry.enabled ? "on" : "off", *entry.interval_ms);
      return;
    }
  }
  notify("unknown log subcommand\n");
}

// ---- start / stop / state / ready / calibrate ----

void cmd_ready(char* args) {
  state.mode = Mode::READY;
  notify("ready\n");
}

void cmd_start(char* args) {
  state.start_time_ms = millis();
  state.countdown_start_ms = millis();
  notify("starting in %lu ms\n", state.start_delay_ms);
  state.mode = Mode::COUNTDOWN;
}

void cmd_stop(char* args) {
  state.mode = Mode::IDLE;
  notify("stopped\n");
}

void cmd_calibrate(char* args) {
  state.mode = Mode::CALIBRATION;
}

void cmd_state(char* args) {
  notify("mode %d kp %.4f ki %.4f kd %.4f k_reverse %.4f\n",
    (int)state.mode, state.pid.kp, state.pid.ki, state.pid.kd, state.k_reverse);
  notify("dist_reverse %d dist_far %d dist_constrain %d\n",
    state.dist_reverse, state.dist_far, state.dist_constrain);
  notify("drive_reversed %d speed_forward %d speed_reverse %d\n",
    state.drive_reversed, state.speed_forward, state.speed_reverse);
#if Is_IMU
  notify("imu_enabled %d slope_boost %d\n", state.imu_enabled, state.slope_boost);
#endif
}

void cmd_help(char* args);

// ---- command table ----

static CommandEntry command_list[] = {
  {"ready",     cmd_ready,     STATE_BIT(Mode::IDLE)},
  {"start",     cmd_start,     STATE_BIT(Mode::READY)},
  {"stop",      cmd_stop,      STATE_BIT(Mode::COUNTDOWN) | STATE_BIT(Mode::RUNNING)},
  {"calibrate", cmd_calibrate, STATE_BIT(Mode::IDLE)},
  {"state",     cmd_state,     ALL_STATES},
  {"log",       cmd_log,       ALL_STATES},
  {"k",         cmd_k,         ALL_STATES},
  {"speed",     cmd_speed,     ALL_STATES},
  {"dist",      cmd_dist,      ALL_STATES},
  {"help",      cmd_help,      ALL_STATES},

  {"delay",     [](char* args){ set_or_report_int(args, &state.start_delay_ms, "start delay %lu ms\n"); }, ALL_STATES},
  {"accel",     [](char* args){ set_or_report_int(args, &state.accel, "accel %d\n"); }, ALL_STATES},
  {"brake",     [](char* args){ set_or_report_int(args, &state.brake, "brake %d\n"); }, ALL_STATES},
  {"slope_t",   [](char* args){ set_or_report_float(args, &state.slope_threshold, "slope threshold %.2f\n"); }, ALL_STATES},

  {"reverse_drive", [](char*){ toggle_bool(&state.drive_reversed, "drive_reversed %d\n"); }, ALL_STATES},
  {"scaled_speed",  [](char*){ toggle_bool(&state.scaled_speed, "scaled_speed %d\n"); }, ALL_STATES},
  {"slope_boost",   [](char*){ toggle_bool(&state.slope_boost, "slope_boost %d\n"); }, ALL_STATES},
  {"imu",           [](char*){ toggle_bool(&state.imu_enabled, "imu_enabled %d\n"); }, ALL_STATES},

#if Memory
  {"save",  [](char*){ state.save_requested = true; }, ALL_STATES},
#endif
  {"180",   [](char*){ state.debug.do_manual_180 = true; }, STATE_BIT(Mode::RUNNING)},
};

void cmd_help(char* args) {
  notify("commands:\n");
  for (auto& entry : command_list) {
    notify("  %s\n", entry.name);
  }
}

void handle_command(char* command, char* args) {
  for (auto& entry : command_list) {
    if (strcmp(command, entry.name) == 0) {
      if (!(entry.allowed_states & STATE_BIT(state.mode))) {
        notify("not allowed in this mode\n");
        return;
      }
      entry.handler(args);
      return;
    }
  }
  notify("unknown command\n");
}

#endif
