#pragma once

/* ============================================================
 *  CONFIGURATION: SET FOR YOUR FOLKRACE TYPE
 * ============================================================
 *  A new user should only need to edit this file for pins,
 *  fitted hardware and feature selection.  Driving logic,
 *  PID and sensor behaviour belong in Android.ino.
 * ============================================================
 */


// Select only one board
#define Is_Arduino     0 // ESP32 firmware only
#define Is_Esp32       1

// Select one or multiple distance-sensor types
#define Is_Sharp       0
#define Is_TOF         0
#define Is_Ultrasonic  0
#define Is_vl53l8cx    0 // not implemented yet

// Select exactly one drive type, and enable servo if needed
#define OneMotor       0
#define TwoMotors      1 // differential drive; can also have a servo
#define tank           0 // four motors
#define Is_servo       0 // steering servo

// Select features
#define Is_blueTooth   1
#define Is_IMU         0
#define Memory         1 //(default 1) does the folk remember parameters?
#define Telemetry      0 //to PC with ground station
#define spaceControl   0 //tries to understand track shape and where it's located on it

// Select ready algorithms
#define Cloude 0 //not implemented yet
#define MyAlgorithm 0 //not implemented yet
#define DefoultAlgorithm 0 //not implemented yet


/* ============================================================
 *  WIRING & CONFIGURATION CHECKLIST
 * ============================================================
 *  Before uploading, verify:
 *    - Pins and motor direction
 *    - I2C addresses
 *    - Sensor lists
 *    - Driver and sensor connections
 * ============================================================
 */

// ---- BLE / command control ----
// The phone connection uses the standard Nordic UART Service.
#define FOLKRACE_BLE_NAME              "Folkrace"
#define FOLKRACE_ENABLE_SERIAL          1
#define FOLKRACE_COMMAND_TIMEOUT_MS     600
#define FOLKRACE_LOOP_PERIOD_MS         10

// ---- Drive tuning ----
// Power values accepted by DRIVE and TANK commands are -255 to 255.
#define FOLKRACE_MAX_POWER              255
#define FOLKRACE_DEAD_BAND              8

// ---- Sensor list ----
// Add or remove a line to add/remove a physical sensor.
// Format: X(name, xshutPin, i2cAddress, angleDegrees)
// angle: 0 = front, 90 = right, -90 = left, 180 = back.
#if Is_TOF
  #define TOF_SENSOR_LIST \
    X(front,      4, 0x30, 0)
#endif

// X(name, pin, angleDegrees)
#if Is_Sharp
  #define SHARP_SENSOR_LIST \
    X(left,       1, -45)   \
    X(right,      2, 45)
#endif

// X(name, echoPin, triggerPin, angleDegrees)
#if Is_Ultrasonic
  #define USONIC_SENSOR_LIST \
    X(leftSide,       3, 6, -90)   \
    X(rightSide,      5, 7, 90)
#endif

// ---- Drive list ----
// DC-motor driver inputs.  The name is also used in Arduino.ino.
// Format: X(name, motorPinA, motorPinB)
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
#endif

// ---- Other sensors ----
#if Is_IMU
  // FastIMU supported device selection and I2C wiring.
  #define IMU_SDA_PIN 21
  #define IMU_SCL_PIN 22
  #define IMU_addres 0x68
  #define IMU_MPU6050 1
  #define IMU_MPU9250 2
  #define IMU_TYPE IMU_MPU6050
#endif

// Compatibility names used by the reusable modules.
#define FOLKRACE_ENABLE_BLE Is_blueTooth
#define FOLKRACE_ENABLE_IMU Is_IMU
