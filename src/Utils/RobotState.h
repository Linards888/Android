#pragma once
#include <Arduino.h>

// ============================================================
//  RobotState.h — the robot's live, tunable data.
// ============================================================
//  Everything in here is a VALUE, not logic. Logic lives in
//  Android.ino and the src/ modules. Almost every number below
//  can be:
//    - read/changed live over BLE (see src/BLE/commands.cpp)
//    - saved to flash and reloaded on boot (see src/Memory)
//    - given a sensible default in Android.ino's "QUICK TUNABLES"
//      block, or in your configs/config_*.h file.
//
//  You will rarely need to add a field here by hand. Most new
//  tunables should instead be registered with ParamRegistry
//  (src/Utils/ParamRegistry.h) — that gives you a BLE command,
//  flash saving, and a `list` printout for free, with one line.
//  Add a field here only when a value needs a *fixed, known*
//  memory address that other C++ code reasons about directly
//  (e.g. things read every control-loop tick).
// ============================================================

// The robot's high-level behaviour state.
enum class RunningState : uint8_t {
    IDLE,             // motors off, waiting for a "ready" command
    READY,            // armed, waiting for "start"
    CALIBRATING,      // sampling sensors, see src/Calibrations
    COUNTDOWN,        // start delay is ticking down
    RUNNING,          // full autonomous line/wall-following logic
    MANUAL_FORWARD,   // debug: drive forward at speed_forward
    MANUAL_BACKWARD,  // debug: drive backward at speed_reverse
};

const char* running_state_name(RunningState s);

// PID gains for the steering control loop.
struct PIDState {
    float kp = 1.0f;
    float ki = 0.0f;
    float kd = 0.0f;
};

// Toggles/intervals for the optional serial debug logging commands
// ("log dist", "log gyro", ...). Not the same thing as Telemetry
// (src/Telemetry), which is the always-on structured PC stream.
struct DebugState {
    bool do_manual_180 = false;

    bool log_distance = false;
    bool log_gyro     = false;
    bool log_accel    = false;
    bool log_yaw      = false;
    bool log_pitch    = false;

    uint16_t log_distance_interval_ms = 200;
    uint16_t log_gyro_interval_ms     = 200;
    uint16_t log_accel_interval_ms    = 200;
    uint16_t log_yaw_interval_ms      = 100;
    uint16_t log_pitch_interval_ms    = 100;

    unsigned long log_distance_last_ms = 0;
    unsigned long log_gyro_last_ms     = 0;
    unsigned long log_accel_last_ms    = 0;
    unsigned long log_yaw_last_ms      = 0;
    unsigned long log_pitch_last_ms    = 0;
};

struct RobotState {
    RunningState running_state = RunningState::IDLE;

    // ---- drive / speed ----
    bool  drive_reversed = false;   // flips motor polarity (robot mounted backwards etc.)
    uint8_t speed_forward = 150;    // base speed while RUNNING            (0-255)
    uint8_t speed_reverse = 150;    // speed used by MANUAL_BACKWARD / escape maneuvers
    uint8_t accel = 4;              // how fast scaled speed ramps up per tick
    uint8_t brake = 10;             // how fast scaled speed ramps down per tick

    // speed scaling by front distance: speed ramps between speed_min
    // (close to an obstacle) and speed_max (far from one).
    bool    scaled_speed = false;
    int16_t dist_near = 0;
    int16_t dist_far  = 0;
    int16_t speed_min = 0;
    int16_t speed_max = 0;

    // "too close" escape maneuver (stop, reverse, swerve).
    uint16_t escape_trigger_mm = 38;
    int16_t  escape_reverse_speed = -50;
    int16_t  escape_turn_speed    = -160;
    uint16_t escape_time_ms       = 200;

    // ---- steering PID ----
    PIDState pid;
    int16_t  pid_output_limit = 255; // clamps the PID steering correction

    // ---- start sequence ----
    uint16_t start_delay_ms = 4500;   // "COUNTDOWN" length after the `start` command
    unsigned long start_time_ms = 0;

    // ---- optional features (see config.h for the on/off switches) ----
    bool imu_enabled = false;
    bool slope_boost = false;
    float k_pitch_running = 0.98f;   // complementary filter gyro-trust weight for pitch (see src/IMU)
    float k_accel_nudge   = 0.01f;   // reserved for future accelerometer-based yaw drift
                                      // correction — registered/saved but not yet applied
    float slope_threshold = 8.0f;    // degrees of pitch that counts as "on a ramp"

    // ---- persistence ----
    bool save_state = false;      // set true (e.g. by the `save` command) to persist to flash
    bool reset_defaults = false;  // set true (by the `defaults` command) to reload the compiled-in
                                   // QUICK TUNABLES from src/Core/RobotLogic.h, without reflashing

    DebugState debug;
};

extern RobotState state;
