#pragma once

/* ============================================================
 *  ⚠ CONFIGURATION: SET FOR YOUR FOLKRACE TYPE ⚠
 * ============================================================
 *  Change the parameters below to match your specific
 *  Folkrace robot type before uploading.
 * ============================================================
 */


//Select only one
#define Is_Arduino     0 //wont work
#define Is_Esp32       1

//Select one or multiple
#define Is_Sharp       0
#define Is_TOF         0
#define Is_Ultrasonic  0
#define Is_vl53l8cx    0 //don't know if i will get this working

//Select only one and servo if necesary
#define OneMotor       0
#define TwoMotors      0 //eather with or without servo
#define tank           0 //4motors
#define Is_servo       0 //for stearing

//Select features
#define Is_blueTooth   0
#define Is_IMU         0
#define Memory         1 //(default 1) does the folk remembers parameters?
#define Telemetry      0 //to PC with ground station
#define SpaceControl   0 //tries to understand track shape and where it's located on it

//Ready library PID algorithms
#define AdvancedPID    0 //by Alby312
#define PID_           0 //by Brett Beauregard






/* ============================================================
 *  ⚠ IMPORTANT: WIRING & CONFIGURATION CHECKLIST ⚠
 * ============================================================
 *  Before uploading, verify ALL of the following:
 *    - Pins        → correct digital/analog pin assignments
 *    - Angles       → servo/motor angle offsets set correctly
 *    - Addresses    → I2C/SPI addresses match your devices
 *    - Names/Labels → variable & device names match hardware
 *    - Connections   → everything wired to the correct place
 * ============================================================
 */



// ---- Sensor list ----
// Add or remove a line to add/remove a physical sensor.
// Format: X(name, xshutPin, i2cAddress, angleDegrees)

  //   angleDegrees: 0 = front, 1-89 = front right sensors, 180 = back, 90 = Right, 179 = back right sensors
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


// X(name, echoPin, trigerPin, angleDegrees)
#if Is_Ultrasonic
  #define USONIC_SENSOR_LIST \
    X(leftSide,       3, 6, -90)   \
    X(rightSide,      5, 7, 90)
#endif


// ---- Drive list ----
//For now only DC motors
// Format: X(name, MotorPinA, MotorPinB)
//   position label is just for your own reference, unused by logic

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

// ---- Other Sensors ----

#if Is_IMU
  #define IMU_INIT_PIN 8
  #define IMU_addres 0x68
#endif