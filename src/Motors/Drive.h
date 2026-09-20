#pragma once
#include <Arduino.h>
#include "config.h"

// Motor driving, abstracted over whichever motor type you picked in
// config.h (DCOneMotor / DCTwoMotors / DCtank / BLDCOneMotor /
// BLDCTwoMotors / BLDCtank), with or without a steering servo (Is_servo).
// config.h already guarantees exactly one motor type is selected before
// this file is even compiled (see the checks in Libraries.h).
//
// No servo library needed: BLDC ESCs and the steering servo both just
// want a 50Hz pulse of a given width, which Drive.cpp generates directly
// with the ESP32's own ledc PWM peripheral (the same one used for the DC
// motor PWM above) - so there's nothing extra to install and nothing
// added to your flash usage beyond what DC driving already needs.
//
// drive()'s signature depends on how many drive motors you actually
// have, since that's how many independent numbers there are to give it:
//
//   - DCOneMotor / BLDCOneMotor:  drive(int speed);
//       One motor, one value, in [-DRIVE_MAX, DRIVE_MAX] (config.h).
//       drive(100)  - full speed ahead.
//       drive(-100) - full speed in reverse.
//
//   - DCTwoMotors / DCtank / BLDCTwoMotors / BLDCtank:
//       drive(int left, int right);
//       One value per side, each in [-DRIVE_MAX, DRIVE_MAX] - true
//       differential drive. (On a tank, both motors on a side get the
//       same value.)
//       drive(100, 100)   - straight ahead.
//       drive(100, -100) - spin in place.
//
// Steering (Is_servo) is always separate from drive(), whichever motor
// type you have - move the servo with steer(delta) whenever you
// want to turn.
//
// Usage:
//   Motorsetup();     // once, in setup()
//   drive(...);  // whichever shape above matches your config
//   steer(60);   // if Is_servo is on - turn right
//   stop();      // motors off (and steering centered)
namespace Drive {
  void setup();
  void stop();

#if DCOneMotor || BLDCOneMotor
  void drive(int speed);
#else
  void drive(int left, int right);
#endif

#if Is_servo
  // Moves the steering servo. delta is in the same [-DRIVE_MAX, DRIVE_MAX]
  // range as drive()'s arguments; it's mapped to [STEER_MIN_DEG,
  // STEER_MAX_DEG] (config.h). Independent of drive() - call it whenever
  // you want to change heading, regardless of motor count.
  void steer(int delta);
#endif
}
