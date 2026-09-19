#pragma once

// ============================================================
//  RobotLogic.h — everything Android.ino needs but shouldn't
//  have to show on its first screen.
// ============================================================
//  Android.ino includes this ONE header and ends up with:
//    - `dt` (delta-time tracker) and `steering_pid` (the PID
//      controller), ready to use
//    - the QUICK TUNABLES defaults + apply_quick_tunables()
//    - registerParams() — the BLE/flash parameter list
//    - init_optional_features() — starts IMU/Memory/BLE/Telemetry/
//      space-awareness, whichever are actually compiled in
//    - the actual RUNNING-state algorithm (run_line_following)
//    - debug-log printing (run_debug_logging)
//    - the "just entered a new state" hook (on_state_enter)
//    - service_tick() — everything loop() needs to do BEFORE the
//      state switch (service sensors/IMU/telemetry/memory, handle
//      the 180/defaults/debug-log housekeeping, fire on_state_enter)
//
//  This file is included exactly once, by Android.ino, so it's
//  fine for it to both declare AND define things — there's only
//  ever one translation unit.
//
//  This is glue and defaults, not architecture — see
//  docs/Arhitecture.md for how each subsystem actually works, and
//  edit the QUICK TUNABLES / registerParams() sections below like
//  you would in Android.ino itself.
// ============================================================

#include "../Utils/RobotState.h"
#include "../Utils/DeltaTime.h"
#include "../Utils/ParamRegistry.h"
#include "../Utils/Notify.h"
#include "../Utils/PIDAlgorithm.h"

#include "../Sensors/DistanceSensors.h"
#include "../drive/Drive.h"
#include "../Calibrations/Calibration.h"

#if FEATURE_BLUETOOTH
    #include "../BLE/RobotBLE.h"
#endif
#if FEATURE_IMU
    #include "../IMU/IMU_logic.h"
#endif
#if FEATURE_MEMORY
    #include "../Memory/StateStore.h"
#endif
#if FEATURE_TELEMETRY
    #include "../Telemetry/Telemetry.h"
#endif
#if FEATURE_SPACE_AWARENESS
    #include "../spaceAverenes/space.h"
#endif
#if SENSOR_TOF_ENABLED || FEATURE_IMU
    #include <Wire.h>
#endif

static DeltaTime dt;
static PIDController steering_pid(state.pid);

// ============================================================
//  QUICK TUNABLES
// ============================================================
//  Defaults for the things you'll change a lot between runs and
//  tracks. These seed `state` at boot (apply_quick_tunables(),
//  called from setup()) — from then on every one of them is
//  live-tunable over BLE ("set kp 1.4") because each has a
//  matching REGISTER_PARAM(...) line below, and, with
//  FEATURE_MEMORY on, remembered across power cycles. Send the
//  `defaults` BLE command any time to snap `state` back to these
//  compiled-in numbers without reflashing. Edit the numbers here
//  when you want a new permanent "factory default".
// ============================================================
namespace tunable {
    constexpr uint16_t START_DELAY_MS = 4500;  // "start" -> countdown -> RUNNING, in ms

    constexpr uint8_t SPEED_FORWARD = 150;     // base driving speed, 0-255
    constexpr uint8_t SPEED_REVERSE = 150;

    constexpr float KP = 1.0f;                 // steering PID gains
    constexpr float KI = 0.0f;
    constexpr float KD = 0.0f;

    constexpr uint8_t ACCEL = 4;               // scaled-speed ramp rates
    constexpr uint8_t BRAKE = 10;
}

static void apply_quick_tunables() {
    state.start_delay_ms = tunable::START_DELAY_MS;
    state.speed_forward  = tunable::SPEED_FORWARD;
    state.speed_reverse  = tunable::SPEED_REVERSE;
    state.pid.kp = tunable::KP;
    state.pid.ki = tunable::KI;
    state.pid.kd = tunable::KD;
    state.accel = tunable::ACCEL;
    state.brake = tunable::BRAKE;
}

// ============================================================
//  Registering BLE/save-able parameters.
// ============================================================
//  To make a new value in RobotState.h tunable over BLE and (if
//  FEATURE_MEMORY is on) persisted to flash, add ONE line here:
//      REGISTER_PARAM(state.my_new_field);
//  or, to give it a short/different name on the wire:
//      REGISTER_PARAM_NAMED("short_name", state.my_new_field);
//  See src/Utils/ParamRegistry.h and docs/Arhitecture.md for the
//  full explanation.
// ============================================================
static void registerParams() {
    REGISTER_PARAM(state.pid.kp);
    REGISTER_PARAM(state.pid.ki);
    REGISTER_PARAM(state.pid.kd);
    REGISTER_PARAM_NAMED("pid_limit", state.pid_output_limit);

    REGISTER_PARAM(state.speed_forward);
    REGISTER_PARAM(state.speed_reverse);
    REGISTER_PARAM(state.accel);
    REGISTER_PARAM(state.brake);
    REGISTER_PARAM(state.scaled_speed);
    REGISTER_PARAM(state.dist_near);
    REGISTER_PARAM(state.dist_far);
    REGISTER_PARAM(state.speed_min);
    REGISTER_PARAM(state.speed_max);

    REGISTER_PARAM_NAMED("esc_trig_mm",  state.escape_trigger_mm);
    REGISTER_PARAM_NAMED("esc_rev_spd",  state.escape_reverse_speed);
    REGISTER_PARAM_NAMED("esc_turn_spd", state.escape_turn_speed);
    REGISTER_PARAM_NAMED("esc_time_ms",  state.escape_time_ms);

    REGISTER_PARAM_NAMED("start_delay", state.start_delay_ms);
    REGISTER_PARAM(state.drive_reversed);

    REGISTER_PARAM(state.imu_enabled);
    REGISTER_PARAM(state.slope_boost);
    REGISTER_PARAM_NAMED("k_pitch_run",  state.k_pitch_running);
    REGISTER_PARAM(state.k_accel_nudge);
    REGISTER_PARAM_NAMED("slope_thresh", state.slope_threshold);

    REGISTER_PARAM(state.debug.log_distance);
    REGISTER_PARAM_NAMED("log_dist_ms", state.debug.log_distance_interval_ms);
    REGISTER_PARAM(state.debug.log_gyro);
    REGISTER_PARAM_NAMED("log_gyro_ms", state.debug.log_gyro_interval_ms);
    REGISTER_PARAM(state.debug.log_accel);
    REGISTER_PARAM_NAMED("log_accel_ms", state.debug.log_accel_interval_ms);
    REGISTER_PARAM(state.debug.log_yaw);
    REGISTER_PARAM_NAMED("log_yaw_ms", state.debug.log_yaw_interval_ms);
    REGISTER_PARAM(state.debug.log_pitch);
    REGISTER_PARAM_NAMED("log_pitch_ms", state.debug.log_pitch_interval_ms);

    sensors_register_params(); // "w_<sensor name>" for every sensor in your config
}

// ============================================================
//  init_optional_features() — starts every optional feature module
//  that's actually compiled in (the ones gated by config.h's
//  FEATURE_* switches). Call once from setup(), after sensors_init()
//  and drive_init() — IMU/telemetry both read sensor/drive state, and
//  BLE needs everything else already up before it starts accepting
//  commands.
// ============================================================
static void init_optional_features() {
#if FEATURE_IMU
    imu_init();
#endif

#if FEATURE_MEMORY
    state_store_init();
    state_store_load(); // overwrites the quick-tunable defaults with saved values, if any
#endif

#if FEATURE_BLUETOOTH
    ble_init();
#endif

#if FEATURE_TELEMETRY
    telemetry_init();
#endif

#if FEATURE_SPACE_AWARENESS
    space_awareness_init();
#endif
}

// ============================================================
//  The RUNNING-state control logic — this is "your algorithm".
// ============================================================
static void run_line_following(float delta_seconds) {
    float front = dist_front();

    // ---- speed: fixed, or scaled by how far the front sensor sees ----
    static int16_t ramped_speed = 0;
    int16_t target_speed = state.speed_forward;

    if (state.scaled_speed && front >= 0) {
        int16_t speed_target;
        if (front >= state.dist_far) {
            speed_target = state.speed_max;
        } else if (front <= state.dist_near) {
            speed_target = state.speed_min;
        } else {
            float s = (front - state.dist_near) / (float)(state.dist_far - state.dist_near);
            speed_target = state.speed_min + (int16_t)((state.speed_max - state.speed_min) * s * s);
        }

        if (speed_target > ramped_speed) ramped_speed = min((int)(ramped_speed + state.accel), (int)speed_target);
        else                              ramped_speed = max((int)(ramped_speed - state.brake), (int)speed_target);
        target_speed = ramped_speed;
    } else {
        ramped_speed = target_speed;
    }

#if FEATURE_IMU
    // simple heuristic: give it a bit more power while pitched up a slope
    if (state.imu_enabled && state.slope_boost && fabsf(imu_pitch()) > state.slope_threshold) {
        target_speed = (int16_t)constrain((int)(target_speed * 1.2f), -255, 255);
    }
#endif

    // ---- too close to something in front? back off and swerve ----
    if (front >= 0 && front < state.escape_trigger_mm) {
        drive_stop();
        delay(150);
        drive_set(state.escape_reverse_speed, state.escape_turn_speed);
        delay(state.escape_time_ms);
        drive_stop();
        delay(150);
        ramped_speed = 0;
        return;
    }

    // ---- steering: weighted sensor sum -> PID -> steer ----
    float error = sensors_steer_error();
    int16_t steer = (int16_t)constrain(steering_pid.update(error, delta_seconds),
                                        (int)-state.pid_output_limit, (int)state.pid_output_limit);

    drive_set(target_speed, steer);
}

static void run_debug_logging() {
    unsigned long now = millis();
    DebugState& d = state.debug;

    if (d.log_distance && now - d.log_distance_last_ms >= d.log_distance_interval_ms) {
        d.log_distance_last_ms = now;
        notify("dist front=%.0f error=%.1f\n", dist_front(), sensors_steer_error());
    }

#if FEATURE_IMU
    if (d.log_gyro && now - d.log_gyro_last_ms >= d.log_gyro_interval_ms) {
        d.log_gyro_last_ms = now;
        notify("gyro x=%.1f y=%.1f z=%.1f\n", imu_gyro_x_dps(), imu_gyro_y_dps(), imu_gyro_z_dps());
    }
    if (d.log_accel && now - d.log_accel_last_ms >= d.log_accel_interval_ms) {
        d.log_accel_last_ms = now;
        notify("accel x=%.2f y=%.2f z=%.2f\n", imu_accel_x_g(), imu_accel_y_g(), imu_accel_z_g());
    }
    if (d.log_yaw && now - d.log_yaw_last_ms >= d.log_yaw_interval_ms) {
        d.log_yaw_last_ms = now;
        notify("yaw %.1f\n", imu_yaw());
    }
    if (d.log_pitch && now - d.log_pitch_last_ms >= d.log_pitch_interval_ms) {
        d.log_pitch_last_ms = now;
        notify("pitch %.1f\n", imu_pitch());
    }
#endif
}

static void on_state_enter(RunningState s) {
    switch (s) {
        case RunningState::CALIBRATING:
            calibration_start();
            break;
        case RunningState::RUNNING:
            steering_pid.reset();
            break;
        default:
            break;
    }
}

// ============================================================
//  service_tick() — everything loop() needs to do every tick,
//  BEFORE deciding what the current state does. Android.ino's
//  loop() just calls this, then runs the state switch.
// ============================================================
static void service_tick(float delta) {
    sensors_update();
#if FEATURE_IMU
    if (state.imu_enabled) imu_update(delta);
#endif
#if FEATURE_TELEMETRY
    telemetry_update();
#endif
#if FEATURE_SPACE_AWARENESS
    space_awareness_update(delta);
#endif

#if FEATURE_MEMORY
    if (state.save_state) {
        state_store_save();
        state.save_state = false;
    }
#endif

    if (state.reset_defaults) {
        apply_quick_tunables();
        steering_pid.reset();
        state.reset_defaults = false;
        notify("defaults applied\n");
    }

    if (state.debug.do_manual_180) {
        drive_do_180();
        state.debug.do_manual_180 = false;
    }

    run_debug_logging();

    // run the right "just entered this state" setup exactly once
    static RunningState previous_state = RunningState::IDLE;
    if (state.running_state != previous_state) {
        on_state_enter(state.running_state);
        previous_state = state.running_state;
    }
}
