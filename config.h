#pragma once

/* ============================================================
 *  CONFIGURATION: SET FOR YOUR FOLKRACE TYPE
 * ============================================================
 *  This is the ONLY file (together with Android.ino) you should
 *  need to touch to run this firmware on your own robot. Change
 *  the values below to match the hardware you actually built,
 *  then upload.
 *
 *  Everything under src/ is a driver/plumbing layer that reads
 *  these flags and lists to decide what to compile in - you
 *  normally never need to open it.
 * ============================================================
 */


// ---- Board ----
// Select only one.
#define Is_Arduino     0 //wont work - this firmware needs ESP32 features (BLE, Preferences, PWM)
#define Is_Esp32       1

// ---- Distance sensors ----
// Select one or multiple. You can mix types (e.g. TOF in front, Sharp on the sides).
#define Is_Sharp       0
#define Is_TOF         0
#define Is_Ultrasonic  0
#define Is_vl53l8cx    0 //not implemented yet - enabling this will fail to compile

// ---- Drive ----
// Select only one drive layout, and Is_servo as well if you steer with a servo
// instead of (or in addition to) differential motor speeds.
#define OneMotor       0
#define TwoMotors      0 //either with or without servo
#define tank           0 //4 motors, skid-steered
#define Is_servo       0 //for steering

// ---- Features ----
#define Is_blueTooth   0
#define Is_IMU         0
#define Memory         1 //(default 1) remember BLE-tuned parameters across reboots
#define Telemetry      0 //not implemented yet - enabling this will fail to compile
#define spaceControl   0 //not implemented yet - enabling this will fail to compile

// ---- Ready-made algorithms ----
// Reserved for future alternate driving algorithms. Leave all at 0 for now -
// the built-in wall-following algorithm in Android.ino is always used.
#define Cloude 0 //not implemented yet
#define MyAlgorithm 0 //not implemented yet
#define DefoultAlgorithm 0 //not implemented yet




/* ============================================================
 *  WIRING & CONFIGURATION CHECKLIST
 * ============================================================
 *  Before uploading, verify ALL of the following:
 *    - Pins        -> correct digital/analog pin assignments
 *    - Angles       -> sensor mounting angle in degrees matches reality
 *    - Addresses    -> I2C addresses match/don't collide with your devices
 *    - Names/Labels -> "front"/"left"/"right" are used consistently; the
 *                      driving logic in Android.ino looks sensors up BY NAME
 *    - Connections  -> everything wired to the correct place
 * ============================================================
 */



// ==== Sensor lists ====
// Add or remove a line to add/remove a physical sensor.
// The names "front", "left" and "right" are special: the wall-following
// logic in Android.ino looks for sensors with exactly these names,
// regardless of which sensor technology provides them. Any other name is
// still read()-able by hand, it's just not used by the built-in algorithm.

//   angleDegrees: 0 = front, 1-89 = front right sensors, 180 = back, 90 = Right, 179 = back right sensors

// Format: X(name, xshutPin, i2cAddress, angleDegrees)
#if Is_TOF
  #define TOF_SENSOR_LIST \
    X(front,      4, 0x30, 0)
#endif


// Format: X(name, analogPin, angleDegrees)
#if Is_Sharp
  #define SHARP_SENSOR_LIST \
    X(left,       1, -45)   \
    X(right,      2, 45)

  // Two-point calibration for whichever Sharp IR distance sensor you have.
  // Sharp sensors are not linear, but a straight line between two measured
  // points is close enough over the ~10-80cm range folkrace robots care
  // about. See docs/Calibration.md for how to measure these two numbers.
  #define SHARP_ADC_AT_NEAR   90    // raw analogRead() with an object at SHARP_NEAR_MM
  #define SHARP_NEAR_MM       100
  #define SHARP_ADC_AT_FAR    520   // raw analogRead() with an object at SHARP_FAR_MM
  #define SHARP_FAR_MM        800
#endif


// Format: X(name, trigPin, echoPin, angleDegrees)
#if Is_Ultrasonic
  #define USONIC_SENSOR_LIST \
    X(leftSide,       6, 3, -90)   \
    X(rightSide,      7, 5, 90)

  #define USONIC_TIMEOUT_US 25000 // ~4m round trip; readings past this count as "nothing there"
#endif


// ==== Drive ====
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

#if Is_servo
  #define SERVO_PIN A3
  #define SERVO_CENTER_DEG 90   // trim so the wheels point straight ahead here
  #define SERVO_MAX_DELTA_DEG 45 // how far the servo may swing off-center
#endif


// ==== Other sensors ====

#if Is_IMU
  #define IMU_INIT_PIN 8   // not currently used by the driver, reserved for an interrupt pin
  #define IMU_addres 0x68

  // Which exact chip FastIMU should talk to. Change this to match your
  // hardware - see the FastIMU library README for the full supported list
  // (examples: MPU6050, MPU6500, MPU9250, MPU9255, ICM20689, LSM6DS3, LSM6DSL, BMI055).
  #define IMU_MODEL MPU6500
#endif

#if Is_blueTooth
  #define BLE_DEVICE_NAME "Folkrace"
#endif


/* ============================================================
 *  TUNABLE DEFAULTS
 * ============================================================
 *  Starting values for everything the BLE console can also change
 *  live (see the "help" command once connected). If MEMORY is on
 *  and you've saved settings over BLE before, the saved values are
 *  used instead of these on every boot after the first - edit these
 *  numbers here for your very first flash, or as a "factory reset"
 *  set of values to go back to.
 * ============================================================
 */

#define DEFAULT_KP              1.0f
#define DEFAULT_KI              0.0f
#define DEFAULT_KD              0.0f
#define DEFAULT_K_LEFT          1.0f
#define DEFAULT_K_LEFT_SIDE     1.0f
#define DEFAULT_K_RIGHT         1.0f
#define DEFAULT_K_RIGHT_SIDE    1.0f
#define DEFAULT_K_REVERSE       1.0f

#define DEFAULT_SPEED_FORWARD   150
#define DEFAULT_SPEED_REVERSE   120
#define DEFAULT_SPEED_MIN       60
#define DEFAULT_SPEED_MAX       255

#define DEFAULT_DIST_NEAR       150   // mm - reserved for future use (e.g. slow-down zone)
#define DEFAULT_DIST_FAR        300   // mm - target distance when following a single wall
#define DEFAULT_DIST_REVERSE    80    // mm - front distance that triggers a reverse+turn
#define DEFAULT_DIST_CONSTRAIN  400   // mm - clamp on the steering error fed into the PID

#define DEFAULT_ACCEL_STEP      10    // max pwm change per control loop tick, speeding up
#define DEFAULT_BRAKE_STEP      30    // max pwm change per control loop tick, slowing down

#define DEFAULT_START_DELAY_MS  3000  // "start" command countdown before RUNNING begins

#define DEFAULT_SLOPE_THRESHOLD 15.0f // degrees of pitch considered "on a ramp"
#define DEFAULT_K_PITCH_RUNNING 0.0f  // extra forward speed per degree of pitch, if slope_boost is on
#define DEFAULT_K_ACCEL_NUDGE   0.0f  // steering correction per unit of lateral accel, if IMU is on

// Non-blocking maneuver timing (front-wall reverse+turn, and the manual "180" debug command)
#define MANEUVER_REVERSE_MS     300
#define MANEUVER_TURN_MS        500
#define MANEUVER_180_MS         900
