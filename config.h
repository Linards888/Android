#pragma once
#include <Arduino.h> // uint8_t/int16_t types + pin names (A0, A1, ...) used by the config structs below

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
 *
 *  Starting tuning values (PID gains, speeds, distances, ...) live
 *  in Defaults.h, not here - reasonable defaults are already filled
 *  in there, and every one of them can also be changed live over
 *  BLE without re-uploading. You don't need to open that file either.
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

// You don't need to understand the "static const struct {...}" part below -
// just copy an existing line inside the { } list and change the values.
// One line = one sensor/motor. Add a line to add one, delete a line to
// remove one.

// ---- TOF (VL53L0X) sensors --------------------------------------------
// One line per sensor: { "name", xshutPin, i2cAddress, angleDegrees }
#if Is_TOF
  static const struct { const char* name; uint8_t xshutPin; uint8_t i2cAddress; int16_t angle; } TOF_SENSORS[] = {
    { "front",      4, 0x30, 0 },
  };
#endif

// ---- Sharp IR sensors ---------------------------------------------------
// One line per sensor: { "name", analogPin, angleDegrees }
#if Is_Sharp
  static const struct { const char* name; uint8_t pin; int16_t angle; } SHARP_SENSORS[] = {
    { "left",       1, -45 },
    { "right",      2, 45 },
  };

  // This sensor's two-point calibration constants (SHARP_ADC_AT_NEAR/FAR,
  // SHARP_NEAR_MM/FAR_MM) live in Defaults.h, not here - see
  // docs/Calibration.md if you need to (re)measure them.
#endif

// ---- Ultrasonic (HC-SR04 style) sensors ---------------------------------
// One line per sensor: { "name", trigPin, echoPin, angleDegrees }
#if Is_Ultrasonic
  static const struct { const char* name; uint8_t trigPin; uint8_t echoPin; int16_t angle; } USONIC_SENSORS[] = {
    { "leftSide",       6, 3, -90 },
    { "rightSide",      7, 5, 90 },
  };

  #define USONIC_TIMEOUT_US 25000 // ~4m round trip; readings past this count as "nothing there"
#endif

// ---- Motors --------------------------------------------------------------
// Only DC motors driven through a 2-pin-per-motor driver (e.g. TB6612,
// DRV8833: one PWM-capable pin per direction, no separate enable pin).
// One line per motor: { "name", motorPinA, motorPinB }

#if OneMotor
  static const struct { const char* name; uint8_t pinA; uint8_t pinB; } MOTORS[] = {
    { "main", 9, 10 },
  };
#endif

#if TwoMotors
  static const struct { const char* name; uint8_t pinA; uint8_t pinB; } MOTORS[] = {
    { "right", 9, 10 },
    { "left", 11, 12 },
  };
#endif

#if tank
  static const struct { const char* name; uint8_t pinA; uint8_t pinB; } MOTORS[] = {
    { "front_right", 9, 10 },
    { "front_left", 11, 12 },
    { "back_right", 13, A0 },
    { "back_left", A1, A2 },
  };
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

// Note: there's no BLE section here - BLE_DEVICE_NAME (the name you'll see
// when scanning) is a starting value, so it lives in Defaults.h instead.


/* ============================================================
 * Starting tuning values + calibration constants live in Defaults.h,
 * automatically included below. You don't need to open that file.
 * ============================================================ */
#include "Defaults.h"
