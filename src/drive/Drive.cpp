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

// Requires an ESP32 Arduino core new enough to provide the classic
// analogWrite() API (arduino-esp32 core 3.x and newer manage the LEDC PWM
// hardware for you). If your board package is older, update it via Boards
// Manager rather than trying to patch this function.
void MotorDrive(Motor* m, int speed) {
  if (m == nullptr) return;

  speed = constrain(speed, -255, 255);

  if (speed >= 0) {
    analogWrite(m->MotorPinA, speed);
    analogWrite(m->MotorPinB, 0);
  } else {
    analogWrite(m->MotorPinA, 0);
    analogWrite(m->MotorPinB, -speed);
  }
}

void stopMotors() {
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    analogWrite(allMotors[i]->MotorPinA, 0);
    analogWrite(allMotors[i]->MotorPinB, 0);
  }
}

/* ---- Usage ----

  motorsetup();

// drive individual motors by name (names come from MOTOR_LIST in config.h)
  MotorDrive(&motor_left, 200);
  MotorDrive(&motor_right, 200);

// or generically, by index
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) MotorDrive(allMotors[i], 150);
*/
