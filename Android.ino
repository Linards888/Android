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
 * ============================================================
 */

#include "Libraries.h"

RobotState state;
static DeltaTime dt;
static PIDController steeringPID;


// ============================================================
//  onRunning() and its helpers - THE DRIVING ALGORITHM
// ============================================================
//
//  Every control loop tick while Mode::RUNNING, in order:
//    1. If a turn maneuver is already in progress (see below), keep
//       driving it and skip everything else this tick.
//    2. Read whichever of the "front" / "left" / "right" sensors
//       config.h defines.
//    3. Front sensor sees a dead end -> start a reverse+turn maneuver.
//    4. Otherwise, work out a steering error from the side sensor(s)
//       and run it through the steering PID.
//    5. Work out a forward speed (optionally boosted on a slope).
//    6. Send speed + steering to the motors/servo.
//
//  Want a different algorithm entirely (line-following instead of
//  wall-following, say)? This is the function to replace - the rest
//  of the file (states, setup/loop) doesn't need to change.

// How far off-center are we? Positive = closer to the right wall than
// the left one, i.e. steer left; the sign convention matches what
// drive_apply()'s `steer` argument expects (positive = turn right).
//
//  - Two side sensors configured: aim to keep them equal, so the robot
//    drives down the middle of the track.
//  - Only one side sensor: hold state.dist_far away from that one wall.
//  - Neither: drive straight (error stays 0).
static int computeSteeringError(int16_t leftDist, int16_t rightDist) {
  int error = 0;

  if (leftDist >= 0 && rightDist >= 0) {
    error = (int)(rightDist * state.pid.k_right_side - leftDist * state.pid.k_left_side);
  } else if (leftDist >= 0) {
    error = (int)((state.dist_far - leftDist) * state.pid.k_left);
  } else if (rightDist >= 0) {
    error = (int)((rightDist - state.dist_far) * state.pid.k_right);
  }

  // Clamp so one glitchy/out-of-range reading can't throw a huge,
  // sudden correction at the steering PID.
  return constrain(error, -state.dist_constrain, state.dist_constrain);
}

// Forward speed for this tick: state.speed_forward, optionally boosted
// while climbing a slope (if Is_IMU + slope_boost are both on), then
// clamped to state.speed_min..state.speed_max.
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
  // 1. A reverse/turn maneuver (or the manual "180" debug command)
  //    takes full control of the motors until it finishes.
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

  // 2. Read sensors. sensor_read() returns -1 for any name config.h
  //    didn't define, regardless of which sensor technology is used.
  int16_t frontDist = sensor_read("front");
  int16_t leftDist  = sensor_read("left");
  int16_t rightDist = sensor_read("right");

  // 3. Dead end ahead: hand off to the reverse+turn maneuver.
  if (frontDist >= 0 && frontDist <= state.dist_reverse) {
    maneuver_start_reverse_turn(leftDist, rightDist);
    maneuver_service();
    return;
  }

  // 4. Steer to stay centered / hold off a single wall.
  int error = computeSteeringError(leftDist, rightDist);
  float steer = steeringPID.update((float)error, dtSeconds);

#if Is_IMU
  // Small steering nudge from lateral acceleration (e.g. drifting sideways
  // on a slippery patch). Zero by default (k_accel_nudge starts at 0).
  if (state.imu_enabled) {
    steer += imu_accel_x() * state.k_accel_nudge;
  }
#endif

  // 5 + 6. Forward speed, then actually drive.
  drive_apply(computeForwardSpeed(), steer);
}


// ============================================================
//  Every other state - short and simple by design
// ============================================================

// Motors off, nothing moving. Only housekeeping BLE commands work here.
static void onIdle() {
  drive_reset();
}

// Armed via the BLE "ready" command, waiting for "start".
static void onReady() {
  drive_reset();
}

// Brief pause after "start", so you have time to let go of the robot
// before it moves. Length is state.start_delay_ms (config.h
// DEFAULT_START_DELAY_MS, or the BLE "delay" command).
static void onCountdown() {
  drive_reset(); // stay stopped while we wait

  if (millis() - state.countdown_start_ms >= state.start_delay_ms) {
    state.mode = Mode::RUNNING;
    steeringPID.reset();
#if Is_blueTooth
    notify("running\n");
#endif
  }
}

// Runs once (see src/Calibrations/Calibration.cpp), then drops back to Idle.
static void onCalibration() {
  run_calibration();
  state.mode = Mode::IDLE;
}


// ============================================================
//  Arduino entry points
// ============================================================

void setup() {
  Serial.begin(115200);

#if Memory
  load_state(); // falls back to config.h DEFAULT_* values if nothing was saved yet
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

  // ---- What does each state do? See the matching function above. ----
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
