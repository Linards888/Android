// ============================================================
//  Android.ino — Folkrace main sketch
// ============================================================
//  This file is meant to stay readable at a glance: what state
//  is the robot in, and what does it do in each state. Wiring,
//  sensors, motors, BLE and every other implementation detail
//  live in src/ and config.h/configs/ — this file just calls the
//  one-line functions those modules expose.
//
//    - Which robot am I building for, what's wired to what:
//        config.h -> configs/config_*.h  (see configs/config_template.h)
//    - Values you'll retune a lot between runs/tracks:
//        the QUICK TUNABLES block right below
//    - How everything fits together:
//        docs/Arhitecture.md
// ============================================================

#include "src/Utils/RobotState.h"
#include "src/Utils/DeltaTime.h"
#include "src/Utils/ParamRegistry.h"
#include "src/Utils/Notify.h"
#include "src/Utils/PIDAlgorithm.h"

#include "src/Sensors/DistanceSensors.h"
#include "src/drive/Drive.h"
#include "src/Calibrations/Calibration.h"

#if FEATURE_BLUETOOTH
    #include "src/BLE/RobotBLE.h"
#endif
#if FEATURE_IMU
    #include "src/IMU/IMU_logic.h"
#endif
#if FEATURE_MEMORY
    #include "src/Memory/StateStore.h"
#endif
#if FEATURE_TELEMETRY
    #include "src/Telemetry/Telemetry.h"
#endif
#if FEATURE_SPACE_AWARENESS
    #include "src/spaceAverenes/space.h"
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
//  tracks. These just seed `state` at boot — from then on every
//  one of them is live-tunable over BLE ("set kp 1.4") and, with
//  FEATURE_MEMORY on, remembered across power cycles. Change the
//  numbers here when you want a new "factory default", not when
//  you're just testing a value on the track (use BLE for that).
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
//  See src/Utils/ParamRegistry.h for the full explanation.
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
//  setup()
// ============================================================
void setup() {
    Serial.begin(115200);
    delay(200); // let USB-serial settle before the first prints

    apply_quick_tunables();
    registerParams();

#if SENSOR_TOF_ENABLED || FEATURE_IMU
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(100000);
#endif

    sensors_init();
    drive_init();

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

    notify("%s ready. Try 'help' over BLE/Serial.\n", ROBOT_NAME);
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
//  loop()
// ============================================================
void loop() {
    dt.update();
    float delta = dt.get();

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

    switch (state.running_state) {
        case RunningState::IDLE:
            // nothing is moving; only BLE commands (ready/get/set/...) do anything
            drive_stop();
            break;

        case RunningState::READY:
            // armed, waiting for `start`
            drive_stop();
            break;

        case RunningState::CALIBRATING:
            if (calibration_run()) {
                state.running_state = RunningState::READY;
            }
            break;

        case RunningState::COUNTDOWN:
            drive_stop();
            if (millis() - state.start_time_ms >= state.start_delay_ms) {
                state.running_state = RunningState::RUNNING;
            }
            break;

        case RunningState::RUNNING:
            run_line_following(delta);
            break;

        case RunningState::MANUAL_FORWARD:
            drive_set(state.speed_forward, 0);
            break;

        case RunningState::MANUAL_BACKWARD:
            drive_set(-(int16_t)state.speed_reverse, 0);
            break;
    }
}
