#include "Drive.h"
#include "Params.h"

namespace {
  int clampSpeed(int v) {
    if (v > DRIVE_MAX) return DRIVE_MAX;
    if (v < -DRIVE_MAX) return -DRIVE_MAX;
    return v;
  }

#if DCOneMotor || DCTwoMotors || DCtank
  void setupDC(const DCMotorPins &m) {
    if (m.pwm != -1) {
      // PWM + direction pin(s).
      ledcAttach(m.pwm, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
      if (m.in1 != -1) pinMode(m.in1, OUTPUT);
      if (m.in2 != -1) pinMode(m.in2, OUTPUT);
    } else {
      // No dedicated PWM pin - driver takes two PWM inputs, one per
      // direction, so both need ledc rather than a plain pinMode().
      if (m.in1 != -1) ledcAttach(m.in1, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
      if (m.in2 != -1) ledcAttach(m.in2, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
    }
  }

  // speed: -DRIVE_MAX..DRIVE_MAX
  void driveDC(const DCMotorPins &m, int speed) {
    speed = clampSpeed(speed);
    bool forward = speed >= 0;
    long pwmMax = (1L << PWM_RESOLUTION_BITS) - 1;
    uint32_t duty = (uint32_t)map(abs(speed), 0, DRIVE_MAX, 0, pwmMax);

    if (m.pwm != -1) {
      if (m.in1 != -1) digitalWrite(m.in1, forward ? HIGH : LOW);
      if (m.in2 != -1) digitalWrite(m.in2, forward ? LOW : HIGH);
      ledcWrite(m.pwm, duty);
    } else {
      // Whichever pin matches the current direction gets the duty
      // cycle, the other stays low.
      if (forward) {
        if (m.in1 != -1) ledcWrite(m.in1, duty);
        if (m.in2 != -1) ledcWrite(m.in2, 0);
      } else {
        if (m.in1 != -1) ledcWrite(m.in1, 0);
        if (m.in2 != -1) ledcWrite(m.in2, duty);
      }
    }
  }
#endif

#if BLDCOneMotor || BLDCTwoMotors || BLDCtank || Is_servo
  // ESCs and hobby servos both run on the same signal: a pulse of a
  // given width (in microseconds), repeated at 50Hz. Rather than pull in
  // a whole servo library just for that, we generate it ourselves with
  // ledc - the same ESP32 PWM peripheral already used for the DC motors
  // above, just on a separate pin/timer at 50Hz instead of 20kHz. This
  // needs a higher bit-depth than the DC motor PWM (16-bit here, vs. the
  // usual 8-bit) purely so a 1000-2000us pulse can still be positioned
  // precisely inside the much longer 20ms (50Hz) period.
  const int SERVO_PWM_FREQ_HZ = 50;
  const int SERVO_PWM_RESOLUTION_BITS = 16;
  const long SERVO_PWM_PERIOD_US = 1000000L / SERVO_PWM_FREQ_HZ;
  const long SERVO_PWM_DUTY_MAX = (1L << SERVO_PWM_RESOLUTION_BITS) - 1;

  void attachServoPin(int pin) {
    ledcAttach(pin, SERVO_PWM_FREQ_HZ, SERVO_PWM_RESOLUTION_BITS);
  }

  void writeServoUs(int pin, int us) {
    uint32_t duty = (uint32_t)((long)us * SERVO_PWM_DUTY_MAX / SERVO_PWM_PERIOD_US);
    ledcWrite(pin, duty);
  }
#endif

#if BLDCOneMotor || BLDCTwoMotors || BLDCtank
  void setupBLDC(const BLDCMotorPins &m) {
    attachServoPin(m.signal);
    writeServoUs(m.signal, ESC_NEUTRAL_US); // arm the ESC at neutral before anything else touches it
  }

  // speed: -DRIVE_MAX..DRIVE_MAX
  void driveBLDC(const BLDCMotorPins &m, int speed) {
    speed = clampSpeed(speed);
    int us = map(speed, -DRIVE_MAX, DRIVE_MAX, ESC_MIN_US, ESC_MAX_US);
    writeServoUs(m.signal, us);
  }
#endif

#if Is_servo
  // angleDeg: 0..180
  void writeServoDeg(int pin, int angleDeg) {
    int us = map(angleDeg, 0, 180, SERVO_PULSE_MIN_US, SERVO_PULSE_MAX_US);
    writeServoUs(pin, us);
  }
#endif
}

namespace Drive {

void Motorsetup() {
#if DCOneMotor
  setupDC(Motor);
#elif DCTwoMotors
  setupDC(MotorLeft);
  setupDC(MotorRight);
#elif DCtank
  setupDC(MotorFrontLeft);
  setupDC(MotorFrontRight);
  setupDC(MotorBackLeft);
  setupDC(MotorBackRight);
#elif BLDCOneMotor
  setupBLDC(Motor);
#elif BLDCTwoMotors
  setupBLDC(MotorLeft);
  setupBLDC(MotorRight);
#elif BLDCtank
  setupBLDC(MotorFrontLeft);
  setupBLDC(MotorFrontRight);
  setupBLDC(MotorBackLeft);
  setupBLDC(MotorBackRight);
#endif

#if Is_servo
  attachServoPin(Steering.signal);
  writeServoDeg(Steering.signal, STEER_CENTER_DEG);
#endif
}

#if DCOneMotor || BLDCOneMotor
void drive(int speed) {
  #if DCOneMotor
    driveDC(Motor, speed);
  #else
    driveBLDC(Motor, speed);
  #endif
}
#else
void drive(int left, int right) {
#if DCTwoMotors
  driveDC(MotorLeft, left);
  driveDC(MotorRight, right);

#elif DCtank
  driveDC(MotorFrontLeft, left);
  driveDC(MotorBackLeft, left);
  driveDC(MotorFrontRight, right);
  driveDC(MotorBackRight, right);

#elif BLDCTwoMotors
  driveBLDC(MotorLeft, left);
  driveBLDC(MotorRight, right);

#elif BLDCtank
  driveBLDC(MotorFrontLeft, left);
  driveBLDC(MotorBackLeft, left);
  driveBLDC(MotorFrontRight, right);
  driveBLDC(MotorBackRight, right);
#endif
}
#endif

void stop() {
#if DCOneMotor || BLDCOneMotor
  drive(0);
#else
  drive(0, 0);
#endif
#if Is_servo
  writeServoDeg(Steering.signal, STEER_CENTER_DEG);
#endif
}

#if Is_servo
void steer(int delta) {
  delta = clampSpeed(delta);
  int angleDeg = map(delta, -DRIVE_MAX, DRIVE_MAX, STEER_MIN_DEG, STEER_MAX_DEG);
  writeServoDeg(Steering.signal, angleDeg);
}
#endif

} // namespace Drive
