#include "Drive.h"

#define X(name, MotorPinA, MotorPinB) Motor motor_##name = { #name, MotorPinA, MotorPinB};
  MOTOR_LIST
#undef X

#define X(name, MotorPinA, MotorPinB) &motor_##name,
  Motor* allMotors[] = { MOTOR_LIST };
#undef X

const uint8_t MOTOR_COUNT = sizeof(allMotors) / sizeof(allMotors[0]);

void motorsetup() {
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    pinMode(allMotors[i]->MotorPinA, OUTPUT);
    pinMode(allMotors[i]->MotorPinB, OUTPUT);
  }
}

void MotorDrive(Motor* m, int speed) {
  if (m == nullptr) return;

  speed = constrain(speed, -255, 255);

  if (speed >= 0) {
    digitalWrite(m->MotorPinA, HIGH);
    digitalWrite(m->MotorPinB, LOW);
  } else {
    digitalWrite(m->MotorPinA, LOW);
    digitalWrite(m->MotorPinB, HIGH);
    speed = -speed;
  }
}

void stopMotors() {
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    digitalWrite(allMotors[i]->MotorPinA, LOW);
    digitalWrite(allMotors[i]->MotorPinB, LOW);
  }
}

/* ---- Usage ----

  motor_setup();

// drive individual motors by name
  motor_setSpeed(&motor_frontLeft, 200);
  motor_setSpeed(&motor_frontRight, 200);
  motor_setSpeed(&motor_backLeft, -200);   // reverse
  motor_setSpeed(&motor_backRight, -200);

// for loop generically
  motor_setSpeed(allMotors[i], 150);
*/