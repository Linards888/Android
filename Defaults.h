#pragma once

/* ============================================================
 *  DEFAULTS - starting values, and constants you shouldn't need
 *  to touch.
 * ============================================================
 *  You do NOT need to open this file to get your robot running -
 *  config.h + Android.ino are enough. This file is automatically
 *  pulled in by config.h, so don't #include it anywhere yourself.
 *
 *  Two different kinds of thing live here:
 *
 *  1. STARTING VALUES - PID gains, speeds, distances, timing.
 *     These can ALSO be changed live over BLE (the "help" command
 *     lists them all) and saved with "save" - saved values then
 *     win over these on every future boot. Treat these as your
 *     robot's starting point / "factory reset" values, not the
 *     only place to tune from. Change them here if you want a
 *     different out-of-the-box starting point.
 *
 *  2. CALIBRATION CONSTANTS (further down) - these are different:
 *     they're measured values from your specific Sharp sensor
 *     hardware, not a preference. Leave them alone unless you've
 *     done the two-point calibration in docs/Calibration.md.
 * ============================================================ */

// ---- BLE ----
#if Is_blueTooth
  #define BLE_DEVICE_NAME "Folkrace" // the name you'll see when scanning for it
#endif

// ---- Steering PID ----
#define DEFAULT_KP              1.0f
#define DEFAULT_KI              0.0f
#define DEFAULT_KD              0.0f
#define DEFAULT_K_LEFT          1.0f  // weight on the left sensor when only it is configured
#define DEFAULT_K_LEFT_SIDE     1.0f  // weight on the left sensor when following down the middle
#define DEFAULT_K_RIGHT         1.0f  // weight on the right sensor when only it is configured
#define DEFAULT_K_RIGHT_SIDE    1.0f  // weight on the right sensor when following down the middle
#define DEFAULT_K_REVERSE       1.0f  // speed multiplier while backing away from a dead end

// ---- Speed (roughly -255..255 PWM units) ----
#define DEFAULT_SPEED_FORWARD   150
#define DEFAULT_SPEED_REVERSE   120
#define DEFAULT_SPEED_MIN       60
#define DEFAULT_SPEED_MAX       255

// ---- Distances (millimeters) ----
#define DEFAULT_DIST_NEAR       150   // reserved for future use (e.g. a slow-down zone)
#define DEFAULT_DIST_FAR        300   // target distance when following a single wall
#define DEFAULT_DIST_REVERSE    80    // front distance that triggers a reverse+turn
#define DEFAULT_DIST_CONSTRAIN  400   // clamp on the steering error fed into the PID

// ---- Speed ramping (pwm units per control loop tick) ----
#define DEFAULT_ACCEL_STEP      10    // max increase per tick, speeding up
#define DEFAULT_BRAKE_STEP      30    // max decrease per tick, slowing down

// ---- Timing ----
#define DEFAULT_START_DELAY_MS  3000  // "start" command countdown before RUNNING begins
#define MANEUVER_REVERSE_MS     300   // how long the reverse+turn maneuver backs up for
#define MANEUVER_TURN_MS        500   // how long it then pivots for
#define MANEUVER_180_MS         900   // how long the manual "180" debug command pivots for

// ---- Slope / IMU-assisted steering (only used if Is_IMU is on) ----
#define DEFAULT_SLOPE_THRESHOLD 15.0f // degrees of pitch considered "on a ramp"
#define DEFAULT_K_PITCH_RUNNING 0.0f  // extra forward speed per degree of pitch, if slope_boost is on
#define DEFAULT_K_ACCEL_NUDGE   0.0f  // steering correction per unit of lateral accel


/* ------------------------------------------------------------
 * CALIBRATION CONSTANTS - don't change these unless you've
 * measured your own Sharp sensor. See docs/Calibration.md.
 * ------------------------------------------------------------ */

// Sharp sensors aren't linear, but a straight line between two measured
// points is close enough over the ~10-80cm range a folkrace track needs.
#if Is_Sharp
  #define SHARP_ADC_AT_NEAR   90    // raw analogRead() with an object at SHARP_NEAR_MM
  #define SHARP_NEAR_MM       100
  #define SHARP_ADC_AT_FAR    520   // raw analogRead() with an object at SHARP_FAR_MM
  #define SHARP_FAR_MM        800
#endif
