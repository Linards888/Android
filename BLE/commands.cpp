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

// _--- k params ----

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
  //ugggggggggggggggghhhhhhhhhhh
}

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
  //i dont want to do it now
}

voidcmd_help(char args) {
  notify(commands:\n);
  for (auto& entry : command_list) {
    notify("  %s\n", entry.name);
  }
}

void handle_command(char* command, char* args) {
  for (auto& entry : command_list) {
    if(strcmp(command, entry.name) == 0) {
      entry.handler(args);
      return;
    }
  }
  notify("unknown commands\n");
}