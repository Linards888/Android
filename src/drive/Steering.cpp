#include "Steering.h"

#if Is_servo
#include <ESP32Servo.h>

static Servo steeringServo;

void steering_setup() {
  steeringServo.attach(SERVO_PIN);
  steering_center();
}

void steering_set(float steer) {
  steer = constrain(steer, -(float)SERVO_MAX_DELTA_DEG, (float)SERVO_MAX_DELTA_DEG);
  steeringServo.write((int)(SERVO_CENTER_DEG + steer));
}

void steering_center() {
  steeringServo.write(SERVO_CENTER_DEG);
}

#endif
