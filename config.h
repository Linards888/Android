#pragma once

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
#define DCTwoMotors     0 //eather with or without servo
#define DCtank          0 //4motors
#define BLDCOneMotor    0
#define BLDCTwoMotors   0 //eather with or without servo
#define BLDCtank        0 //4motors
#define Is_servo        0 //for stearing

//Select features
#define Is_blueTooth   0
#define Is_IMU         0
#define Telemetry      0 //to PC with ground station
#define spaceControl   0 //tries to understand track shape and where it's located on it

//Select ready Algorithms
#define Cloude 0      //not implemented yet
#define MyAlgorithm 0 //not implemented yet



const float ADC_MAX = 4095.0;  // ESP32 ADC is 12-bit
const float ADC_VREF = 3.3;    // ESP32 ADC reference voltage, in volts


// ---- Sensor list ----
struct TOFSensor {
  uint8_t pin;
  uint16_t address;
  int16_t angle;
};

struct SharpSensor {
  uint8_t pin;
  int16_t angle;
};


// ---- Sensor instances ----
// Name = {XSHUT_PIN, I2C_ADDRESS, ANGLE_DEGREES}
TOFSensor Front     = {8, 0x30, 0};
TOFSensor Right     = {9, 0x31, 45};
TOFSensor Left      = {10, 0x32, -45};
TOFSensor RightSide = {11, 0x33, 90};
TOFSensor LeftSide  = {12, 0x34, -90};


// Name = {ANALOG_PIN, ANGLE_DEGREES}
SharpSensor SharpFront     = {-1, 0};
SharpSensor SharpRight     = {-1, 45};
SharpSensor SharpLeft      = {-1, -45};
SharpSensor SharpRightSide = {-1, 90};
SharpSensor SharpLeftSide  = {-1, -90};


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
  DCMotorPins Motor = {25, 26, 27};

#elif DCTwoMotors
  DCMotorPins MotorLeft  = {25, 26, 27};
  DCMotorPins MotorRight = {14, 32, 33};

#elif DCtank
  DCMotorPins MotorFrontLeft  = {25, 26, 27};
  DCMotorPins MotorFrontRight = {14, 32, 33};
  DCMotorPins MotorBackLeft   = {17, 16, 4};
  DCMotorPins MotorBackRight  = {21, 19, 18};

#elif BLDCOneMotor
  // Name = {SIGNAL_PIN}
  BLDCMotorPins Motor = {25};

#elif BLDCTwoMotors
  BLDCMotorPins MotorLeft  = {25};
  BLDCMotorPins MotorRight = {26};

#elif BLDCtank
  BLDCMotorPins MotorFrontLeft  = {25};
  BLDCMotorPins MotorFrontRight = {26};
  BLDCMotorPins MotorBackLeft   = {27};
  BLDCMotorPins MotorBackRight  = {14};
#endif

#if Is_servo
  SteeringPins Steering = {13};
#endif


// ---- Drive tuning ----
// Range for Drive::drive()'s left/right arguments (and Drive::steer()'s
// delta) - e.g. drive(100, 100) is full speed straight ahead,
// drive(-100, -100) is full speed in reverse, drive(100, -100) spins in
// place on a differential config. Keep this in mind if you feed PID
// output straight into drive() - clamp/scale it to this range first.
const int DRIVE_MAX = 255;

// DC motor PWM (ledc) settings.
const int PWM_FREQ_HZ = 20000;      // 20kHz - above human hearing
const int PWM_RESOLUTION_BITS = 8;  // 0-255 duty steps

// BLDC ESC pulse widths, in microseconds (standard RC PWM/PPM). Re-tune
// these against your own ESC's calibration if it doesn't follow the
// 1000/1500/2000us convention.
const int ESC_MIN_US = 1000;      // full reverse (or full-off, on ESCs with no reverse)
const int ESC_NEUTRAL_US = 1500;  // stop
const int ESC_MAX_US = 2000;      // full forward

// Steering servo angle range, in degrees.
const int STEER_MIN_DEG = 45;
const int STEER_CENTER_DEG = 90;
const int STEER_MAX_DEG = 135;

// Pulse width (microseconds) corresponding to 0 and 180 degrees - most
// hobby servos land somewhere in this range. Narrow it if yours buzzes
// or strains at the endpoints Drive.cpp actually commands
// (STEER_MIN_DEG..STEER_MAX_DEG above).
const int SERVO_PULSE_MIN_US = 500;   // 0 degrees
const int SERVO_PULSE_MAX_US = 2500;  // 180 degrees
