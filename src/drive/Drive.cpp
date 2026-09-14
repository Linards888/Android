#include "Drive.h"

#if FOLKRACE_MOTOR_COUNT != 2
  #error "This starter drive supports two motors. Add custom multi-motor logic in Android.ino."
#endif

static Motor leftMotor = {"left", FOLKRACE_LEFT_IN1, FOLKRACE_LEFT_IN2, FOLKRACE_LEFT_REVERSED};
static Motor rightMotor = {"right", FOLKRACE_RIGHT_IN1, FOLKRACE_RIGHT_IN2, FOLKRACE_RIGHT_REVERSED};
Motor* allmotors[] = {&leftMotor, &rightMotor};
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
void driveTank(int leftPower, int rightPower) { setMotor(*allmotors[0], leftPower); setMotor(*allmotors[1], rightPower); }

void driveArcade(int throttle, int steering) {
  throttle = constrain(throttle, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
  steering = constrain(steering, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
  driveTank(constrain(throttle + steering, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER),
            constrain(throttle - steering, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER));
}

void stopMotors() { for (uint8_t i = 0; i < MOTOR_COUNT; ++i) setMotor(*allmotors[i], 0); }
