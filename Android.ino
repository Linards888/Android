/* ============================================================
 *  Android.ino - Folkrace main sketch
 * ============================================================
 *  Hardware selection, pins and addresses live in config.h. Starting
 *  tuning values and calibration constants live in Defaults.h.
 *  "Libraries.h" below pulls in every other library/driver your
 *  config.h selections actually need (motors, sensors, IMU, etc.) -
 *  see src/Utils/Libraries.h if you're curious how.
 *
 *  THIS file is the robot's actual behaviour. It's organized as one
 *  function per state:
 *
 *      onIdle        - motors off, just sitting there
 *      onReady       - armed, waiting for the BLE "start" command
 *      onCountdown   - short delay after "start" (state.start_delay_ms,
 *                      DEFAULT_START_DELAY_MS in Defaults.h), before it goes
 *      onCalibration - runs once, then drops back to Idle
 *      onForward     - bench-test: drive straight forward
 *      onBackwards   - bench-test: drive straight backward
 *      onRunning     - actually drives the track - YOU write this one
 *
 *  loop() just asks "which state are we in?" and calls the matching
 *  function. To change what a state does, edit its function - you
 *  don't need to touch anything else.
 *
 *  onRunning() is intentionally blank. Read the sensors with
 *  sensor_read(), decide on a steering value and a speed however you
 *  like (a PID loop, if/else rules, whatever you want to write), and
 *  hand the result to drive_apply(). See docs/Algorithm.md for a full
 *  worked example (PID wall-following) you can study or paste in as a
 *  starting point, and docs/Android_ino.md for the full toolbox of
 *  functions you can call from here.
 * ============================================================
 */

#include "DeltaTime.h"
#include "RobotState.h"
#include "Libraries.h"
#include "RobotBLE.h"
#include "Calibration.h"

RobotState state;
static DeltaTime dt;
static PIDController steeringPID; // handy if you want a PID loop - see onRunning() below


// ---- onRunning() - THIS IS WHERE YOUR DRIVING ALGORITHM GOES ----
// Nothing is implemented here on purpose - write your own sensor reading,
// steering algorithm (PID or otherwise) and drive calls. See
// docs/Algorithm.md for a complete worked example.
static void onRunning(float dtSeconds) {
  // 1. Read whichever sensors you named in config.h, e.g.:
  //      int16_t frontDist = sensor_read("front");
  //      int16_t leftDist  = sensor_read("left");
  //      int16_t rightDist = sensor_read("right");
  //    Each call returns millimeters, or -1 if that sensor isn't configured.

  // 2. Decide on a steering correction (positive = turn right). If you want
  //    a PID loop, `steeringPID` is already declared above:
  //      float steer = steeringPID.update(error, dtSeconds);

  // 3. Decide on a forward speed, e.g. state.speed_forward, clamped to
  //    state.speed_min..state.speed_max.

  // 4. Drive:
  //      drive_apply(forwardSpeed, steer);
}


// ---- Manual bench-test states - triggered over BLE, see docs/BLE_Commands.md ----

static void onForward() {
  drive_apply(state.speed_forward, 0);
}

static void onBackwards() {
  drive_apply(-state.speed_reverse, 0);
}


// ---- Every other state - see docs/Arhitecture.md ----

static void onIdle() {
  // Nothing is running, no motors, no driving logic - the robot just sits
  // here. The only BLE commands that work are housekeeping ones (state,
  // help, tuning, forward/backward/calibrate) until you send "ready".
  drive_reset();
}

static void onReady() {
  // Armed and waiting for the BLE "start" command.
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
  Serial.println("Calibrating sensors...");
  run_calibration();
  state.mode = Mode::IDLE;
}


// ---- Arduino entry points ----

void setup() {
  Serial.begin(115200);

#if Memory
  load_state(); // reads any previously-saved BLE tuning values from flash
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
    case Mode::FORWARD:     onForward();     break;
    case Mode::BACKWARDS:   onBackwards();   break;
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
