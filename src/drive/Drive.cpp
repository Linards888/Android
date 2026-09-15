#include "Drive.h"

#define X(name, pinA, pinB) Motor motor_##name = {#name, pinA, pinB, false};
  MOTOR_LIST
#undef X

#define X(name, pinA, pinB) &motor_##name,
Motor* allmotors[] = { MOTOR_LIST };
#undef X
const uint8_t MOTOR_COUNT = sizeof(allmotors) / sizeof(allmotors[0]);

static int limitPower(int power) {
  power = constrain(power, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
  return abs(power) < FOLKRACE_DEAD_BAND ? 0 : power;
}

void driveBegin() {
  for (uint8_t i = 0; i < MOTOR_COUNT; ++i) {
    pinMode(allmotors[i]->in1, OUTPUT);
    pinMode(allmotors[i]->in2, OUTPUT);
  }
  stopMotors();
}

void setMotor(Motor& motor, int power) {
  power = limitPower(power);
  if (motor.reversed) power = -power;
  if (power > 0) {
    analogWrite(motor.in1, power);
    digitalWrite(motor.in2, LOW);
  } else if (power < 0) {
    digitalWrite(motor.in1, LOW);
    analogWrite(motor.in2, -power);
  } else {
    digitalWrite(motor.in1, LOW);
    digitalWrite(motor.in2, LOW); // coast; use HIGH/HIGH here if the driver brakes
  }
}

void setMotor(uint8_t index, int power) { if (index < MOTOR_COUNT) setMotor(*allmotors[index], power); }
void driveTank(int leftPower, int rightPower) {
#if TwoMotors
  setMotor(motor_left, leftPower);
  setMotor(motor_right, rightPower);
#else
  // Define your own mapping in Android.ino for one-motor or four-motor cars.
  (void)leftPower; (void)rightPower;
#endif
}

void driveArcade(int throttle, int steering) {
  driveTank(constrain(throttle + steering, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER),
            constrain(throttle - steering, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER));
}

void stopMotors() { for (uint8_t i = 0; i < MOTOR_COUNT; ++i) setMotor(*allmotors[i], 0); }
