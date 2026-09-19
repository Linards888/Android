// ============================================================
//  Android.ino — Folkrace main sketch
// ============================================================
//  This file is meant to stay readable at a glance: what does
//  setup() switch on, and what does the robot do in each state.
//  Wiring, sensors, motors, BLE, and every other implementation
//  detail live in src/ and config.h/configs/. The default values
//  you'll retune a lot, and the parameter list that makes them
//  BLE-tunable, live in src/Core/RobotLogic.h — see the comment
//  at its top before you go looking for "QUICK TUNABLES" here.
//
//    - Which robot am I building for, what's wired to what:
//        config.h -> configs/config_*.h  (see configs/config_template.h)
//    - Values you'll retune a lot between runs/tracks, and the
//      BLE parameter list:
//        src/Core/RobotLogic.h
//    - How everything fits together:
//        docs/Arhitecture.md
// ============================================================

#include "src/Core/RobotLogic.h"

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

    init_optional_features(); // IMU/Memory/BLE/Telemetry/space-awareness — see src/Core/RobotLogic.h

    notify("%s ready. Try 'help' over BLE/Serial.\n", ROBOT_NAME);
}

// ============================================================
//  loop()
// ============================================================
//  service_tick() (src/Core/RobotLogic.h) does everything that
//  needs to happen every tick regardless of state — servicing
//  sensors/IMU/telemetry, saving to flash, the 180/defaults/debug-
//  log commands, and firing on_state_enter() when the state just
//  changed. What's left here is exactly "what state am I in, what
//  do I do about it".
// ============================================================
void loop() {
    dt.update();
    float delta = dt.get();

    service_tick(delta);

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
