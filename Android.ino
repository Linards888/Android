/* ============================================================
 *  Android.ino - Folkrace main sketch
 * ============================================================
 *  Hardware selection, pins, addresses and starting tuning values
 *  live in config.h - that's the other file you edit. Everything
 *  under src/ is drivers/plumbing (motor PWM, sensor reading, BLE
 *  wiring, flash storage) that this file calls into but you
 *  normally don't need to open.
 *
 *  THIS file is the robot's actual behaviour. It's organized as
 *  one function per state:
 *
 *      onIdle        - motors off, just sitting there
 *      onReady       - armed, waiting for the BLE "start" command
 *      onCountdown   - short delay after "start", before it goes
 *      onCalibration - runs once, then drops back to Idle
 *      onRunning     - actually drives the track (see below)
 *
 *  loop() just asks "which state are we in?" and calls the matching
 *  function. To change what a state does, edit its function - you
 *  don't need to touch anything else.
 *
 *  onRunning() is the one you'll come back to most: it's the whole
 *  track-following algorithm, in one place, read top to bottom.
 *  See docs/Algorithm.md for a full walkthrough of what it does and
 *  why, and docs/Arhitecture.md for how this file fits together
 *  with everything under src/.
 * ============================================================
 */

#include "Libraries.h"

RobotState state;
static DeltaTime dt;
static PIDController steeringPID;


// ---- onRunning() and its helpers - see docs/Algorithm.md ----

static int computeSteeringError(int16_t leftDist, int16_t rightDist) {
  int error = 0;

  if (leftDist >= 0 && rightDist >= 0) {
    error = (int)(rightDist * state.pid.k_right_side - leftDist * state.pid.k_left_side);
  } else if (leftDist >= 0) {
    error = (int)((state.dist_far - leftDist) * state.pid.k_left);
  } else if (rightDist >= 0) {
    error = (int)((rightDist - state.dist_far) * state.pid.k_right);
  }

  return constrain(error, -state.dist_constrain, state.dist_constrain);
}

static int computeForwardSpeed() {
  int speed = state.speed_forward;

#if Is_IMU
  if (state.imu_enabled && state.slope_boost && imu_pitch() > state.slope_threshold) {
    speed += (int)(state.k_pitch_running * imu_pitch());
  }
#endif

  return constrain(speed, state.speed_min, state.speed_max);
}

static void onRunning(float dtSeconds) {
  if (maneuver_active()) {
    maneuver_service();
    return;
  }
  if (state.debug.do_manual_180) {
    state.debug.do_manual_180 = false;
    maneuver_start_180();
    maneuver_service();
    return;
  }

  int16_t frontDist = sensor_read("front");
  int16_t leftDist  = sensor_read("left");
  int16_t rightDist = sensor_read("right");

  if (frontDist >= 0 && frontDist <= state.dist_reverse) {
    maneuver_start_reverse_turn(leftDist, rightDist);
    maneuver_service();
    return;
  }

  int error = computeSteeringError(leftDist, rightDist);
  float steer = steeringPID.update((float)error, dtSeconds);

#if Is_IMU
  if (state.imu_enabled) {
    steer += imu_accel_x() * state.k_accel_nudge;
  }
#endif

  drive_apply(computeForwardSpeed(), steer);
}


// ---- Every other state - see docs/Arhitecture.md ----

static void onIdle() {
  drive_reset();
}

static void onReady() {
  drive_reset();
}

static void onCountdown() {
  drive_reset();

  if (millis() - state.countdown_start_ms >= state.start_delay_ms) {
    state.mode = Mode::RUNNING;
    steeringPID.reset();
#if Is_blueTooth
    notify("running\n");
#endif
  }
}

static void onCalibration() {
  run_calibration();
  state.mode = Mode::IDLE;
}


// ---- Arduino entry points ----

void setup() {
  Serial.begin(115200);

#if Memory
  load_state();
#endif

  motorsetup();
#if Is_servo
  steering_setup();
#endif

  sensors_setup();

#if Is_IMU
  imu_setup();
#endif

#if Is_blueTooth
  initBLE();
#endif

  Serial.println("Folkrace ready.");
}

void loop() {
  dt.update();
  float dtSeconds = dt.getSeconds();

#if Is_IMU
  if (state.imu_enabled) imu_update(dtSeconds);
#endif

  switch (state.mode) {
    case Mode::IDLE:        onIdle();        break;
    case Mode::READY:       onReady();       break;
    case Mode::COUNTDOWN:   onCountdown();   break;
    case Mode::CALIBRATION: onCalibration(); break;
    case Mode::RUNNING:
      steeringPID.configure(state.pid.kp, state.pid.ki, state.pid.kd);
      onRunning(dtSeconds);
      break;
  }

#if Is_blueTooth
  service_debug_logs();
#endif

#if Memory
  if (state.save_requested) {
    save_state();
#if Is_blueTooth
    notify("saved\n");
#endif
    state.save_requested = false;
  }
#endif
}
