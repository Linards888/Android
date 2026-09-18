#include "Drive.h"
#include "RobotState.h"

#if Is_servo
  #include "Steering.h"
#endif

const uint8_t MOTOR_COUNT = sizeof(MOTORS) / sizeof(MOTORS[0]);
Motor allMotors[MOTOR_COUNT];

// Finds a motor by the name it was given in config.h's MOTORS[] list.
Motor* motor_getByName(const char* name) {
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    if (strcmp(allMotors[i].name, name) == 0) return &allMotors[i];
  }
  return nullptr;
}

// Checks that every motor name the selected drive layout needs (see
// drive_apply() below) is actually present in config.h's MOTORS[] list, and
// prints a clear message instead of silently doing nothing if one's missing.
static void checkRequiredMotor(const char* name) {
  if (motor_getByName(name) == nullptr) {
    Serial.print("Drive: missing required motor \"");
    Serial.print(name);
    Serial.println("\" - check the MOTORS[] list in config.h");
  }
}

void motorsetup() {
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
    allMotors[i] = { MOTORS[i].name, MOTORS[i].pinA, MOTORS[i].pinB };
    pinMode(allMotors[i].MotorPinA, OUTPUT);
    pinMode(allMotors[i].MotorPinB, OUTPUT);
  }

#if OneMotor
  checkRequiredMotor("main");
#elif TwoMotors
  checkRequiredMotor("left");
  checkRequiredMotor("right");
#elif tank
  checkRequiredMotor("front_left");
  checkRequiredMotor("front_right");
  checkRequiredMotor("back_left");
  checkRequiredMotor("back_right");
#endif
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
    analogWrite(allMotors[i].MotorPinA, 0);
    analogWrite(allMotors[i].MotorPinB, 0);
  }
}

/* ---- Usage ----

  motorsetup();

// drive individual motors by name (names come from MOTORS[] in config.h)
  MotorDrive(motor_getByName("left"), 200);
  MotorDrive(motor_getByName("right"), 200);

// or generically, by index
  for (uint8_t i = 0; i < MOTOR_COUNT; i++) MotorDrive(&allMotors[i], 150);
*/


// ==== High-level drive control ====
// (see Drive.h for what each of these does and why)

static int rampSpeed(int current, int target, int accelStep, int brakeStep) {
  if (target > current) return min(target, current + accelStep);
  if (target < current) return max(target, current - brakeStep);
  return current;
}

#if OneMotor
  static int currentSpeed = 0;
#else
  static int currentLeft = 0;
  static int currentRight = 0;
#endif

static void resetDriveRamp() {
#if OneMotor
  currentSpeed = 0;
#else
  currentLeft = 0;
  currentRight = 0;
#endif
}

void drive_apply(int forwardSpeed, float steer) {
  if (state.drive_reversed) {
    forwardSpeed = -forwardSpeed;
    steer = -steer;
  }

#if Is_servo
  steering_set(steer);
  int leftTarget = forwardSpeed;
  int rightTarget = forwardSpeed;
#else
  int leftTarget  = forwardSpeed - (int)steer;
  int rightTarget = forwardSpeed + (int)steer;
#endif

#if OneMotor
  static Motor* motor_main = motor_getByName("main");
  currentSpeed = rampSpeed(currentSpeed, forwardSpeed, state.accel, state.brake);
  MotorDrive(motor_main, currentSpeed);
#elif TwoMotors
  static Motor* motor_left  = motor_getByName("left");
  static Motor* motor_right = motor_getByName("right");
  currentLeft  = rampSpeed(currentLeft,  leftTarget,  state.accel, state.brake);
  currentRight = rampSpeed(currentRight, rightTarget, state.accel, state.brake);
  MotorDrive(motor_left,  currentLeft);
  MotorDrive(motor_right, currentRight);
#elif tank
  static Motor* motor_front_left  = motor_getByName("front_left");
  static Motor* motor_back_left   = motor_getByName("back_left");
  static Motor* motor_front_right = motor_getByName("front_right");
  static Motor* motor_back_right  = motor_getByName("back_right");
  currentLeft  = rampSpeed(currentLeft,  leftTarget,  state.accel, state.brake);
  currentRight = rampSpeed(currentRight, rightTarget, state.accel, state.brake);
  MotorDrive(motor_front_left,  currentLeft);
  MotorDrive(motor_back_left,   currentLeft);
  MotorDrive(motor_front_right, currentRight);
  MotorDrive(motor_back_right,  currentRight);
#endif
}

void drive_reset() {
  stopMotors();
#if Is_servo
  steering_center();
#endif
  resetDriveRamp();
  state.maneuver = Maneuver::NONE;
}


// ==== Front-wall reverse+turn / manual "180" maneuver ====

static unsigned long maneuverTurnMs = MANEUVER_TURN_MS;

bool maneuver_active() {
  return state.maneuver != Maneuver::NONE;
}

void maneuver_start_reverse_turn(int16_t leftDist, int16_t rightDist) {
  // Turn toward whichever side currently has more room. If we don't have
  // both side readings, keep whatever direction was used last time.
  if (leftDist >= 0 && rightDist >= 0) {
    state.turn_direction = (leftDist > rightDist) ? -1 : 1;
  }
  state.maneuver = Maneuver::REVERSING;
  state.maneuver_start_ms = millis();
}

void maneuver_start_180() {
  maneuverTurnMs = MANEUVER_180_MS;
  state.maneuver = Maneuver::TURNING;
  state.maneuver_start_ms = millis();
}

void maneuver_service() {
  unsigned long elapsed = millis() - state.maneuver_start_ms;

  switch (state.maneuver) {
    case Maneuver::REVERSING:
      drive_apply(-(int)(state.speed_reverse * state.k_reverse), 0);
      if (elapsed >= MANEUVER_REVERSE_MS) {
        maneuverTurnMs = MANEUVER_TURN_MS;
        state.maneuver = Maneuver::TURNING;
        state.maneuver_start_ms = millis();
      }
      break;

    case Maneuver::TURNING: {
      int turnSpeed = state.speed_forward * state.turn_direction;
      drive_apply(0, (float)turnSpeed);
      if (elapsed >= maneuverTurnMs) {
        state.maneuver = Maneuver::NONE;
        resetDriveRamp();
      }
      break;
    }

    default:
      break;
  }
}
