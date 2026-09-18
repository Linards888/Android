/* ============================================================
 *  Android.ino - Folkrace main sketch
 * ============================================================
 *  This file holds the robot's actual behaviour: the state
 *  machine, the wall-following drive algorithm, and wiring the
 *  optional features (BLE console, IMU, memory) together.
 *
 *  Hardware selection and pin/address/tuning numbers live in
 *  config.h - that's the other file you should need to edit.
 *  Everything under src/ is drivers/plumbing that config.h and
 *  this file call into.
 * ============================================================
 */

#include "Libraries.h"

RobotState state;
static DeltaTime dt;
static PIDController steeringPID;


// ---- Drive helpers ----

// Moves `current` toward `target` by at most `accelStep` (speeding up) or
// `brakeStep` (slowing down) per call, so speed changes ramp instead of
// jumping instantly - gentler on gearboxes and grip.
static int rampSpeed(int current, int target, int accelStep, int brakeStep) {
  if (target > current) return min(target, current + accelStep);
  if (target < current) return max(target, current - brakeStep);
  return current;
}

#if OneMotor
static int currentSpeed = 0;
#else
static int currentLeft = 0;
static int currentRight = 0;
#endif

static void resetDriveRamp() {
#if OneMotor
  currentSpeed = 0;
#else
  currentLeft = 0;
  currentRight = 0;
#endif
}

// Turns a desired forward speed + steering correction into actual motor (and
// servo, if fitted) commands, ramping speed changes via state.accel/brake.
static void drive_apply(int forwardSpeed, float steer) {
  if (state.drive_reversed) {
    forwardSpeed = -forwardSpeed;
    steer = -steer;
  }

#if Is_servo
  steering_set(steer);
  int leftTarget = forwardSpeed;
  int rightTarget = forwardSpeed;
#else
  int leftTarget  = forwardSpeed - (int)steer;
  int rightTarget = forwardSpeed + (int)steer;
#endif

#if OneMotor
  currentSpeed = rampSpeed(currentSpeed, forwardSpeed, state.accel, state.brake);
  MotorDrive(&motor_main, currentSpeed);
#elif TwoMotors
  currentLeft  = rampSpeed(currentLeft,  leftTarget,  state.accel, state.brake);
  currentRight = rampSpeed(currentRight, rightTarget, state.accel, state.brake);
  MotorDrive(&motor_left,  currentLeft);
  MotorDrive(&motor_right, currentRight);
#elif tank
  currentLeft  = rampSpeed(currentLeft,  leftTarget,  state.accel, state.brake);
  currentRight = rampSpeed(currentRight, rightTarget, state.accel, state.brake);
  MotorDrive(&motor_front_left,  currentLeft);
  MotorDrive(&motor_back_left,   currentLeft);
  MotorDrive(&motor_front_right, currentRight);
  MotorDrive(&motor_back_right,  currentRight);
#endif
}


// ---- Front-wall reverse+turn / manual "180" maneuver ----
// Implemented as a short, timed, non-blocking sequence serviced once per
// loop() iteration, instead of a blocking delay() that would freeze BLE and
// sensor reads for the duration of the turn.

static unsigned long maneuverTurnMs = MANEUVER_TURN_MS;

static void start_reverse_turn(int16_t leftDist, int16_t rightDist) {
  // Turn toward whichever side currently has more room. If we don't have
  // both side readings, keep whatever direction was used last time.
  if (leftDist >= 0 && rightDist >= 0) {
    state.turn_direction = (leftDist > rightDist) ? -1 : 1;
  }
  state.maneuver = Maneuver::REVERSING;
  state.maneuver_start_ms = millis();
}

static void start_manual_180() {
  maneuverTurnMs = MANEUVER_180_MS;
  state.maneuver = Maneuver::TURNING;
  state.maneuver_start_ms = millis();
}

static void service_maneuver() {
  unsigned long elapsed = millis() - state.maneuver_start_ms;

  switch (state.maneuver) {
    case Maneuver::REVERSING:
      drive_apply(-(int)(state.speed_reverse * state.k_reverse), 0);
      if (elapsed >= MANEUVER_REVERSE_MS) {
        maneuverTurnMs = MANEUVER_TURN_MS;
        state.maneuver = Maneuver::TURNING;
        state.maneuver_start_ms = millis();
      }
      break;

    case Maneuver::TURNING: {
      int turnSpeed = state.speed_forward * state.turn_direction;
      drive_apply(0, (float)turnSpeed);
      if (elapsed >= maneuverTurnMs) {
        state.maneuver = Maneuver::NONE;
        resetDriveRamp();
      }
      break;
    }

    default:
      break;
  }
}


// ---- The actual wall-following algorithm ----
//
//  - If the front sensor sees a wall closer than dist_reverse, back up and
//    pivot toward whichever side has more room.
//  - Otherwise, if both a left and right sensor are configured, steer to
//    equalize their distances (drive down the middle of the track).
//  - If only one side sensor is configured, steer to hold dist_far away
//    from that single wall.
//  - Optionally (IMU enabled), boost forward speed on an uphill slope and
//    nudge steering from lateral acceleration.
static void drive_step(float dtSeconds) {
  if (state.maneuver != Maneuver::NONE) {
    service_maneuver();
    return;
  }

  if (state.debug.do_manual_180) {
    state.debug.do_manual_180 = false;
    start_manual_180();
    service_maneuver();
    return;
  }

  int16_t frontDist = sensor_read("front");
  int16_t leftDist  = sensor_read("left");
  int16_t rightDist = sensor_read("right");

  if (frontDist >= 0 && frontDist <= state.dist_reverse) {
    start_reverse_turn(leftDist, rightDist);
    service_maneuver();
    return;
  }

  int error = 0;
  if (leftDist >= 0 && rightDist >= 0) {
    error = (int)(rightDist * state.pid.k_right_side - leftDist * state.pid.k_left_side);
  } else if (leftDist >= 0) {
    error = (int)((state.dist_far - leftDist) * state.pid.k_left);
  } else if (rightDist >= 0) {
    error = (int)((rightDist - state.dist_far) * state.pid.k_right);
  }
  error = constrain(error, -state.dist_constrain, state.dist_constrain);

  float steer = steeringPID.update((float)error, dtSeconds);

  int forwardSpeed = state.speed_forward;

#if Is_IMU
  if (state.imu_enabled) {
    if (state.slope_boost && imu_pitch() > state.slope_threshold) {
      forwardSpeed += (int)(state.k_pitch_running * imu_pitch());
    }
    steer += imu_accel_x() * state.k_accel_nudge;
  }
#endif

  forwardSpeed = constrain(forwardSpeed, state.speed_min, state.speed_max);

  drive_apply(forwardSpeed, steer);
}


// ---- Periodic BLE telemetry (the "log" command) ----

#if Is_blueTooth
static unsigned long lastLogDistance = 0, lastLogGyro = 0, lastLogAccel = 0, lastLogYaw = 0, lastLogPitch = 0;

static void service_debug_logs() {
  unsigned long now = millis();

  if (state.debug.log_distance && now - lastLogDistance >= state.debug.log_distance_interval_ms) {
    lastLogDistance = now;
    notify("dist f:%d l:%d r:%d\n", sensor_read("front"), sensor_read("left"), sensor_read("right"));
  }

#if Is_IMU
  if (state.debug.log_gyro && now - lastLogGyro >= state.debug.log_gyro_interval_ms) {
    lastLogGyro = now;
    notify("gyro x:%.2f y:%.2f z:%.2f\n", imu_gyro_x(), imu_gyro_y(), imu_gyro_z());
  }
  if (state.debug.log_accel && now - lastLogAccel >= state.debug.log_accel_interval_ms) {
    lastLogAccel = now;
    notify("accel x:%.2f y:%.2f z:%.2f\n", imu_accel_x(), imu_accel_y(), imu_accel_z());
  }
  if (state.debug.log_yaw && now - lastLogYaw >= state.debug.log_yaw_interval_ms) {
    lastLogYaw = now;
    notify("yaw %.2f\n", imu_yaw());
  }
  if (state.debug.log_pitch && now - lastLogPitch >= state.debug.log_pitch_interval_ms) {
    lastLogPitch = now;
    notify("pitch %.2f\n", imu_pitch());
  }
#endif
}
#endif


// ---- Arduino entry points ----

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

  switch (state.mode) {
    case Mode::IDLE:
      stopMotors();
#if Is_servo
      steering_center();
#endif
      resetDriveRamp();
      break;

    case Mode::CALIBRATION:
      run_calibration();
      state.mode = Mode::IDLE;
      break;

    case Mode::READY:
      stopMotors();
      break;

    case Mode::COUNTDOWN:
      stopMotors();
      if (millis() - state.countdown_start_ms >= state.start_delay_ms) {
        state.mode = Mode::RUNNING;
        steeringPID.reset();
        resetDriveRamp();
#if Is_blueTooth
        notify("running\n");
#endif
      }
      break;

    case Mode::RUNNING:
      steeringPID.configure(state.pid.kp, state.pid.ki, state.pid.kd);
      drive_step(dtSeconds);
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
