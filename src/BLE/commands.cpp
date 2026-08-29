#include "commands.h"
#include <FastIMU.h>
#include "RobotState.h"
#include <Arduino.h>

extern RobotState state;
extern calData imu_cal_data;
extern void notify(const char* fmt, ...);

//Generic helpers

template<typename T>
static void set_or_report_int(char*& args, T* target, const char* fmt) {
  char* value = strtok_r(args, " \r\n", &args);
  if (value != NULL) *target = static_cast<T>(atol(value));
  notify(fmt, *target);
}

static void set_or_report_float(char*& args, float* target, const char* fmt) {
  char* value = strtok_r(args, " \r\n", &args);
  if (value != NULL) *target = atoff(value);
  notify(fmt, *target);
}

static void toggle_bool(bool* target, const char* fmt) {
  *target = !*target;
  notify(fmt, *target);
}

// ---- k params ----

struct FloatEntry { const char* name; float* target; const char* fmt; };

struct FloatEntry k_list[] = {
  {"p",               &state.pid.kp, "kp %.4f\n"},
  {"i",               &state.pid.ki, "ki %.4f\n"},
  {"d",               &state.pid.kd, "kd %.4f\n"},
  {"reverse",         &state.k_reverse, "k_reverse %.4f\n"},
  {"left_side",       &state.pid.k_left_side, "k_left_side %.4f\n"},
  {"left",            &state.pid.k_left, "k_left %.4f\n"},
  {"right_side",      &state.pid.k_right_side, "k_right %.4\n"},
  {"right",           &state.pid.k_right, "k_right %.4f\n"},
  {"pitch_running",   &state.k_pitch_running, "k_pitch_running %.4f\n"},
  {"accel_nudge",     &state.k_accel_nudge, "_accel_nudge %.4f\n"},
};

void cmd_k(char* args) {
  char* argument = strtok_r(args, " \r\n", &args);
  if (argument == NULL) {
    notify("k_reverse %.4f\nk_left_side %.4f\nk_left %.4f\nk_right %.4\nk_right_side %.4f\n"),
      state.k_reverse, state.pid.k_left_side, state.pid.k_left, state.pid.k_right, state.pid.k_right_side);
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
      notify("scaled speed %d\nspeed min %d\nspeed max %d\nforward speed %d\nreverse speed %d\n,",
        state.scaled_speed, state.state_min, state.speed_max, state.speed_forward, state.speed_reverse);
      return;
    }
    if      (strcmp(argument, "forward") == 0) set_or_report_int(args, &state.speed_forward,  "forward speed %d\n");
    if else (strcmp(argument, "reverse") == 0) set_or_report_int(args, &state.speed_reverse,  "reverse speed %d\n");
    if else (strcmp(argument, "min") == 0)     set_or_report_int(args, &state.speed_min,      "speed min %d\n");
    if else (strcmp(argument, "max") == 0)     set_or_report_int(args, &state.speed_max,      "speed max %d\n");
}


// ---- dist----

void cmd_dist(char* args) {
  char* argument = strtok_r(args, " \r\n", &args);
  if (argument == NULL) {
        notify("reverse distance %d\nnear distance %d\nfar distance %d\n", state.dist_reverse, state.dist_near, state.dist_far);
        return;
  }
  if        (strcmp(argument, "reverse") == 0)  set_or_report_int(args, &state.dist_reverse,  "reverse distance %d\n");
  else if   (strcmp(argument, "far") == 0)      set_or_report_int(args, &state.dist_far,      "far distance %d\n");
}

// ---- Log ----

struct LogEntry { const char* name; bool* enabled; uint16_t* interval_ms; const char* label; };

struct LogEntry log_list[] = {
  {"dist", &state.debug.log_distance, &state.debug.log_distance_interval_ms,  "distance"},
  {"gyro", &state.debug.log_gyro,     &state.debug.log_gyro_interval_ms,      "gyro"},
  {"accel", &state.debug.log_accel,   &state.debug.log_accel_interval_ms,     "accel"},
  {"yaw", &state.debug.log_yaw,       &state.debug.log_yaw_interval_ms,       "yaw"},
  {"pitch", &state.debug.log_pitch,   &state.debug.log_pitch_interval_ms,     "pitch"}
};

void cmd_log(char* args) {
  char* argument = strtok_r(args, " \r\n", &args);
  if (argument == NULL) { notify("no argument provided to log\n"); return; }

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

// ---- start / stop / state ----

void cmd_start(char* args) {
  state.start_time_ms = millis();
  if (state.imu_enabled) {
    imu_cal_data = {0};
    notify("calibrating imu...\n");
  }
  notify("starting in %d ms\n", state.start_delay_ms);
  state.running_state = RunningState::COUNTDOWN;
}

void cmd_stop(char* args){
  state.running_state = RunningState::IDLE;
  notify("stoping");
}

void cmd_state(char* args){
    notify("kp %.4f\nki %.4f\nkd %.4f\nk_reverse %.4f\ndistance constrain %d\nreverse distance %d\n",
            state.pid.kp, state.pid.ki, state.pid.kd, state.k_reverse, state.dist_constrain, state.dist_reverse);
    notify("imu %d\ndrive direction %d\nforward speed %d\nreverse speed %d\n",
            state.imu_enabled, state.drive_reversed, state.speed_forward, state.speed_reverse);
}
void cmd_help(char* args);

// ---- command table ----

CommandEntry command_list[] = {
  {"ready", cmd_ready, STATE_BIT(RunningState::IDLE)},  //only form IDLE

  {"start", cmd_start,  STATE_BIT(RunningState::READY)},
  {"stop", cmd_stop,    STATE_BIT(RunningState::COUNTDOWN) | STATE_BIT(RunningState::RUNNING)},
  {"state", cmd_state,  ALL_STATES},
  {"log", cmd_log,      ALL_STATES},
  {"k", cmd_k,          ALL_STATES},
  {"speed", cmd_speed,  ALL_STATES},
  {"dist", cmd_dist,    ALL_STATES},
  {"help", cmd_help,    ALL_STATES},

  {"accel",     [](char* args){ set_or_report_int(args, &state.accel, "accel %d\n"); }, ALL_STATES},
  {"brake",     [](char* args){ set_or_report_int(args, &state.brake, "brake %d\n"); }, ALL_STATES},
  {"constrain", [](char* args){ set_or_report_int(args, &state.dist_constrain, "constrain %d\n"); }, ALL_STATES},
  {"slope_t",   [](char* args){ set_or_report_float(args, &state.slope_threshold, "slope treshold %.2f\n"); }, ALL_STATES},

  {"reverse_drive", [](char*){ toggle_bool(&state.drive_reverse, "drive direction %d\n"); }, ALL_STATES},
  {"scaled_speed",  [](char*){ toggle_bool(&state.scaled_speed, "scaled_speed %d\n"); }, ALL_STATES},
  {"slope_boost",   [](char*){ toggle_bool(&state.slope_boost, "slope_boost %d\n"); },  ALL_STATES},
  {"imu",           [](char*){ toggle_bool(&state.imu_enable, "imu %d\n"); },   ALL_STATES},

  {"save",  [](char*){ state.save_state = true; }, ALL_STATES},
  {"180",   [](char*){ state.debug.do_manual_180 = true }, STATE_BIT(RunningState::RUNNING)},
};

void cmd_help(char args) {
  notify(commands:\n);
  for (auto& entry : command_list) {
    notify("  %s\n", entry.name);
  }
}

void cmd_ready(char* args) {
    state.running_state = RunningState::READY;
    notify("ready\n");
}

void handle_command(char* command, char* args) {
  for (auto& entry : command_list) {
    if(strcmp(command, entry.name) == 0) {
      if (!(entry.allowed_states & STATE_BIT(state.running_state))) {
        notify("not allowed while in this state\n");
        return;
      }
      entry.handler(args);
      return;
    }
  }
  notify("unknown command\n");
}