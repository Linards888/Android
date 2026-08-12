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

//Select one and servo if necesary
#define OneMotor       0
#define TwoMotors      0 //eather with or without servo
#define tank           0 //4motors
#define servo          0 //for stearing

//Select features
#define Is_blueTooth   0
#define Is_IMU         0
#define Memory         1 //(default 1) does the folk remembers parameters?
#define Telemetry      0 //to PC with ground station

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



// ---- TOF sensor list ----
// Add or remove a line to add/remove a physical sensor.
// Format: X(name, xshutPin, i2cAddress, angleDegrees)
//   angleDegrees: 0 = front, -1-(-89) && 1-89 = front sensors, 180 = back, 90 = Right
#if Is_TOF
  #define TOF_SENSOR_LIST \
    X(front,      4, 0x30, 0)   \
    X(right,      7, 0x33, 90)
#endif