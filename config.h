#pragma once

/* ============================================================
 *  CONFIGURATION - describe the robot you actually built
 * ============================================================
 *  This is the ONLY file (together with Android.ino) you should
 *  need to touch to run this firmware on your own robot.
 *  Everything under src/ just reads the flags and lists below to
 *  decide what to compile in - you normally never need to open it.
 *
 *  Fill it in, in this order:
 *    1. HARDWARE - board, sensors, drive, optional features (below)
 *    2. WIRING - pins/addresses/angles for the sensors and motors
 *       you just turned on, further down this file
 *    3. STARTING TUNING VALUES - PID gains, speeds, distances, all
 *       the way at the bottom - reasonable defaults are already
 *       filled in, and every one of them can also be changed live
 *       over BLE later without re-uploading
 *
 *  Leaving everything below at 0 is intentional: the build will
 *  refuse to compile with a clear message telling you what to set,
 *  rather than silently producing a robot that can't drive.
 * ============================================================
 */


/* ------------------------------------------------------------
 * 1. HARDWARE - what does your robot actually have?
 * ---------------------------------------------------------- */

// ---- Board: select exactly one ----
#define Is_Arduino     0 // wont work - this firmware needs ESP32 features (BLE, Preferences, PWM)
#define Is_Esp32       1

// ---- Distance sensors: select one or more (you can mix types) ----
#define Is_Sharp       0 // analog IR distance sensors
#define Is_TOF         0 // VL53L0X time-of-flight sensors
#define Is_Ultrasonic  0 // HC-SR04 style trigger/echo sensors

// ---- Drive: select exactly one layout ----
#define OneMotor       0 // single drive motor - needs Is_servo to be able to steer at all
#define TwoMotors      0 // two independently-driven motors, steer by differential speed
#define tank           0 // four motors (skid-steer), steer by differential speed
#define Is_servo       0 // add a steering servo (usable with any of the layouts above)

// ---- Optional features ----
#define Is_blueTooth   0 // BLE tuning console - see docs/Setup.md for the command list
#define Is_IMU         0 // gyro/accelerometer, used for slope boost + steering nudge
#define Memory         1 // (default 1) remember BLE-tuned parameters across reboots

// ---- Not implemented yet - leave these at 0 ----
// Turning any of these on fails the build on purpose, so you don't spend
// time wondering why a "supported" feature silently does nothing.
#define Is_vl53l8cx    0
#define Telemetry      0
#define spaceControl   0


/* ------------------------------------------------------------
 * 2. WIRING - pins, I2C addresses, mounting angles
 * ------------------------------------------------------------
 *  Checklist before you upload:
 *    - Pins        -> match your actual wiring
 *    - Angles      -> sensor mounting angle in degrees, matches reality
 *    - Addresses   -> I2C addresses don't collide with each other
 *    - Names       -> see the note below - "front"/"left"/"right" are special
 * ---------------------------------------------------------- */

// The names "front", "left" and "right" are special: the wall-following
// algorithm in Android.ino looks for sensors with exactly these names,
// regardless of which sensor technology provides them (mix and match freely,
// e.g. a TOF "front" with two Sharp "left"/"right"). Any other name still
// works if you read it by hand, it's just not used by the built-in algorithm.
//   angleDegrees: 0 = front, 1-89 = front-right, 90 = right, 180 = back, -1..-89 = front-left, -90 = left

// ---- TOF (VL53L0X) sensors --------------------------------------------
// Format: X(name, xshutPin, i2cAddress, angleDegrees)
#if Is_TOF
  #define TOF_SENSOR_LIST \
    X(front,      4, 0x30, 0)
#endif

// ---- Sharp IR sensors ---------------------------------------------------
// Format: X(name, analogPin, angleDegrees)
#if Is_Sharp
  #define SHARP_SENSOR_LIST \
    X(left,       1, -45)   \
    X(right,      2, 45)

  // Two-point calibration (see docs/Calibration.md for how to measure these):
  // Sharp sensors aren't linear, but a straight line between two measured
  // points is close enough over the ~10-80cm range a folkrace track needs.
  #define SHARP_ADC_AT_NEAR   90    // raw analogRead() with an object at SHARP_NEAR_MM
  #define SHARP_NEAR_MM       100
  #define SHARP_ADC_AT_FAR    520   // raw analogRead() with an object at SHARP_FAR_MM
  #define SHARP_FAR_MM        800
#endif

// ---- Ultrasonic (HC-SR04 style) sensors ---------------------------------
// Format: X(name, trigPin, echoPin, angleDegrees)
#if Is_Ultrasonic
  #define USONIC_SENSOR_LIST \
    X(leftSide,       6, 3, -90)   \
    X(rightSide,      7, 5, 90)

  #define USONIC_TIMEOUT_US 25000 // ~4m round trip; readings past this count as "nothing there"
#endif

// ---- Motors --------------------------------------------------------------
// Only DC motors driven through a 2-pin-per-motor driver (e.g. TB6612,
// DRV8833: one PWM-capable pin per direction, no separate enable pin).
// Format: X(name, MotorPinA, MotorPinB)

#if OneMotor
  #define MOTOR_LIST \
    X(main, 9, 10)
#endif

#if TwoMotors
  #define MOTOR_LIST \
    X(right, 9, 10) \
    X(left, 11, 12)
#endif

#if tank
  #define MOTOR_LIST \
    X(front_right, 9, 10) \
    X(front_left, 11, 12) \
    X(back_right, 13, A0) \
    X(back_left, A1, A2)
#endif

// ---- Steering servo -------------------------------------------------------
#if Is_servo
  #define SERVO_PIN A3
  #define SERVO_CENTER_DEG 90    // trim so the wheels point straight ahead here
  #define SERVO_MAX_DELTA_DEG 45 // how far the servo may swing off-center
#endif

// ---- IMU -------------------------------------------------------------------
#if Is_IMU
  #define IMU_addres 0x68

  // Which exact chip FastIMU should talk to - change this to match your
  // hardware. See the FastIMU library README for the full supported list
  // (examples: MPU6050, MPU6500, MPU9250, MPU9255, ICM20689, LSM6DS3, LSM6DSL, BMI055).
  #define IMU_MODEL MPU6500
#endif

// ---- BLE ---------------------------------------------------------------
#if Is_blueTooth
  #define BLE_DEVICE_NAME "Folkrace" // this is the name you'll see when scanning for it
#endif


/* ============================================================
 * 3. STARTING TUNING VALUES
 * ============================================================
 *  Everything below can ALSO be changed live over BLE (the "help"
 *  command lists them all) and saved with "save" - saved values then
 *  win over these on every future boot. Treat these as your robot's
 *  starting point / "factory reset" values, not the only place to
 *  tune from.
 * ============================================================ */

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
