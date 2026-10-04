#pragma once
#include <stdint.h>

/* ============================================================
 *  CONFIGURATION: SET FOR YOUR FOLKRACE TYPE
 * ============================================================
 *  Change the parameters below to match your specific
 *  Folkrace robot type before uploading.
 * ============================================================
 */

#define Is_Esp32       1

//Select one or multiple
#define Is_Sharp       0
#define Is_TOF         0
#define Is_vl53l8cx    0 //don't know if i will get this working

//Select only one and servo if necesary
#define DCOneMotor      0
#define DCTwoMotors     1 //eather with or without servo
#define DCtank          0 //4motors
#define BLDCOneMotor    0
#define BLDCTwoMotors   0 //eather with or without servo
#define BLDCtank        0 //4motors
#define Is_servo        0 //for stearing

//Select features
#define Is_blueTooth   1
#define Is_IMU         0
#define Telemetry      0 //to PC with ground station
#define spaceControl   0 //tries to understand track shape and where it's located on it

//Select ready Algorithms
#define Cloude 0      //not implemented yet
#define MyAlgorithm 0 //not implemented yet



const float ADC_MAX = 4095.0;  // ESP32 ADC is 12-bit
const float ADC_VREF = 3.3;    // ESP32 ADC reference voltage, in volts


// ---- Pin rules (ESP32 DevKit / WROOM-32) ----
// Every global below is `inline` so this header can be included from
// several .cpp files without "multiple definition" link errors, and so
// every file sees the same object (Tof::read()/Sharp::read() match
// sensors by address). Use -1 for "not connected".
//
//   NEVER use 6-11    - wired to the on-module flash, the chip crashes.
//   Avoid 0, 2, 12, 15 - boot strapping pins; 12 held HIGH at power-on
//                       stops the board from booting.
//   21 / 22           - default I2C SDA / SCL, used by the ToF sensors.
//   34-39             - input only (no pull-ups) - fine for Sharp sensors.
//   Analog (Sharp)    - use ADC1 pins (32-39); ADC2 stops working while
//                       WiFi is on.


// ---- Sensor list ----
struct TOFSensor {
  int8_t pin;
  uint16_t address;
  int16_t angle;
};

struct SharpSensor {
  int8_t pin;
  int16_t angle;
};


// ---- Sensor instances ----
// Name = {XSHUT_PIN, I2C_ADDRESS, ANGLE_DEGREES}
inline TOFSensor Front     = {13, 0x30, 0};
inline TOFSensor Right     = {16, 0x31, 45};
inline TOFSensor Left      = {17, 0x32, -45};
inline TOFSensor RightSide = {18, 0x33, 90};
inline TOFSensor LeftSide  = {19, 0x34, -90};


// Name = {ANALOG_PIN, ANGLE_DEGREES}
// Free ADC1 pins on a DevKit: 34, 35, 36, 39 (32/33 are ADC1 too, but
// the default MotorRight uses them).
inline SharpSensor SharpFront     = {-1, 0};
inline SharpSensor SharpRight     = {-1, 45};
inline SharpSensor SharpLeft      = {-1, -45};
inline SharpSensor SharpRightSide = {-1, 90};
inline SharpSensor SharpLeftSide  = {-1, -90};


// ---- Motor pins ----
// Only the struct(s) matching your motor selection above actually get
// used - see src/Motors/Drive.h for the drive(left, right) API that
// works the same way regardless of which type you picked.
//
// DC motors - driven through an H-bridge (L298N, TB6612, DRV8833, ...):
//   pwm - PWM speed pin. Set to -1 if your driver takes two PWM inputs
//         instead of a single PWM + direction pins (e.g. some DRV8871/
//         TB6612-style boards driven IN1=PWM/IN2=PWM) - then wire speed
//         through in1/in2 directly and Drive.cpp will PWM those instead.
//   in1/in2 - direction pins. Leave in2 at -1 if your driver only needs
//         a single direction pin (PWM + DIR style).
struct DCMotorPins {
  int8_t pwm;
  int8_t in1;
  int8_t in2;
};

// BLDC motors - driven through an ESC that takes a standard RC PWM/PPM
// signal (a 1000-2000us pulse), exactly like a servo.
struct BLDCMotorPins {
  int8_t signal;
};

// Steering servo (used when Is_servo is on).
struct SteeringPins {
  int8_t signal;
};

#if DCOneMotor
  // Name = {PWM_PIN, IN1_PIN, IN2_PIN}
  inline DCMotorPins Motor = {25, 26, 27};

#elif DCTwoMotors
  inline DCMotorPins MotorLeft  = {25, 26, 27};
  inline DCMotorPins MotorRight = {14, 32, 33};

#elif DCtank
  inline DCMotorPins MotorFrontLeft  = {25, 26, 27};
  inline DCMotorPins MotorFrontRight = {14, 32, 33};
  // 4 motors use most of the free pins: the back motors share 16-19
  // with the default ToF XSHUT pins above. Using ToF with a tank, either
  // run fewer ToF sensors or wire each back motor in parallel with the
  // front motor on the same side (they always get the same speed).
  inline DCMotorPins MotorBackLeft   = {23, 16, 4};
  inline DCMotorPins MotorBackRight  = {5, 19, 18};

#elif BLDCOneMotor
  // Name = {SIGNAL_PIN}
  inline BLDCMotorPins Motor = {25};

#elif BLDCTwoMotors
  inline BLDCMotorPins MotorLeft  = {25};
  inline BLDCMotorPins MotorRight = {26};

#elif BLDCtank
  inline BLDCMotorPins MotorFrontLeft  = {25};
  inline BLDCMotorPins MotorFrontRight = {26};
  inline BLDCMotorPins MotorBackLeft   = {27};
  inline BLDCMotorPins MotorBackRight  = {14};
#endif

#if Is_servo
  inline SteeringPins Steering = {23};
#endif
