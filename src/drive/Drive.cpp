#include "Drive.h"

#define X(name, MotorPinA, MotorPinB) Motor motor_##name = {#name, MotorPinA, MotorPinB};
  MOTOR_LIST
#undef X

#define X(name, MotorPinA, MotorPinB) &motor_##name,
Motor* allmotors[] = { MOTOR_LIST };
#undef X

const uint8_t MOTOR_COUNT = sizeof(allmotors) / sizeof(allmotors[0]);

void motorsetup() {
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    pinMode(allmotors[i]->MotorPinA, OUTPUT);
    pinMode(allmotors[i]->MotorPinB, OUTPUT);
  }
  stopMotors();
}

void MotorDrive(Motor* m, int speed) {
  if (m == nullptr) return;
  speed = constrain(speed, -255, 255);

  if (speed > 0) {
    analogWrite(m->MotorPinA, speed);
    digitalWrite(m->MotorPinB, LOW);
  } else if (speed < 0) {
    digitalWrite(m->MotorPinA, LOW);
    analogWrite(m->MotorPinB, -speed);
  } else {
    digitalWrite(m->MotorPinA, LOW);
    digitalWrite(m->MotorPinB, LOW);
  }
}

void stopMotors() {
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    MotorDrive(allmotors[i], 0);
  }
}
