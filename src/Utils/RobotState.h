#pragma once
#include <Arduino.h>
#include "config.h"

// ---- Top-level operating mode ----
//
//   IDLE        motors off, sensors idle. Only housekeeping BLE commands work.
//   READY       armed, waiting for the "start" command.
//   COUNTDOWN   short delay after "start", before RUNNING begins.
//   RUNNING     actually driving the track - this is where YOUR algorithm goes.
//   CALIBRATION runs the (re)calibration routine once, then returns to IDLE.
//   FORWARD     bench-test: drives straight forward at speed_forward.
//   BACKWARDS   bench-test: drives straight backward at speed_reverse.
//
enum class Mode : uint8_t {
  IDLE = 0,
  READY,
  COUNTDOWN,
  RUNNING,
  CALIBRATION,
  FORWARD,
  BACKWARDS
};

// Bitmask helpers used to say which modes a BLE command is allowed in.
#define STATE_BIT(m) (uint8_t)(1 << (uint8_t)(m))
#define ALL_STATES   (uint8_t)0xFF

// While driving, a "maneuver" temporarily overrides the normal wall-following
// math with a short, timed, non-blocking sequence (used for the front-wall
// reverse+turn, and the manual "180" debug command).
enum class Maneuver : uint8_t {
  NONE = 0,
  REVERSING,
  TURNING
};

struct PidParams {
  float kp = DEFAULT_KP;
  float ki = DEFAULT_KI;
  float kd = DEFAULT_KD;
  float k_left       = DEFAULT_K_LEFT;
  float k_left_side  = DEFAULT_K_LEFT_SIDE;
  float k_right      = DEFAULT_K_RIGHT;
  float k_right_side = DEFAULT_K_RIGHT_SIDE;
};

struct DebugFlags {
  bool log_distance = false;
  bool log_gyro     = false;
  bool log_accel    = false;
  bool log_yaw      = false;
  bool log_pitch    = false;

  uint16_t log_distance_interval_ms = 250;
  uint16_t log_gyro_interval_ms     = 250;
  uint16_t log_accel_interval_ms    = 250;
  uint16_t log_yaw_interval_ms      = 250;
  uint16_t log_pitch_interval_ms    = 250;

  bool do_manual_180 = false; // set by the BLE "180" command, cleared once handled
};

// All of the robot's runtime-tunable and runtime-only state lives here in one
// place, so the BLE console, Memory (save/load) and the driving logic in
// Android.ino all agree on what a field is called.
struct RobotState {
  Mode mode = Mode::IDLE;

  PidParams pid;
  float k_reverse = DEFAULT_K_REVERSE;

  int speed_forward = DEFAULT_SPEED_FORWARD;
  int speed_reverse = DEFAULT_SPEED_REVERSE;
  int speed_min     = DEFAULT_SPEED_MIN;
  int speed_max     = DEFAULT_SPEED_MAX;
  bool scaled_speed = false;

  int dist_near      = DEFAULT_DIST_NEAR;
  int dist_far       = DEFAULT_DIST_FAR;
  int dist_reverse   = DEFAULT_DIST_REVERSE;
  int dist_constrain = DEFAULT_DIST_CONSTRAIN;

  bool drive_reversed = false;
  int accel = DEFAULT_ACCEL_STEP;
  int brake = DEFAULT_BRAKE_STEP;

  bool imu_enabled = false;
  float k_pitch_running = DEFAULT_K_PITCH_RUNNING;
  float k_accel_nudge   = DEFAULT_K_ACCEL_NUDGE;
  float slope_threshold = DEFAULT_SLOPE_THRESHOLD;
  bool slope_boost = false;

  bool save_requested = false; // set by the BLE "save" command, serviced in loop()

  unsigned long start_time_ms     = 0;
  unsigned long start_delay_ms    = DEFAULT_START_DELAY_MS;
  unsigned long countdown_start_ms = 0;

  Maneuver maneuver = Maneuver::NONE;
  unsigned long maneuver_start_ms = 0;
  int8_t turn_direction = 1; // +1 = turn right, -1 = turn left

  DebugFlags debug;
};

// Defined once, in Android.ino.
extern RobotState state;
