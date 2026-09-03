#include "FolkraceRuntime.h"

namespace {
int commandInt(const String& command, int start, int fallback) {
  if (command.length() <= start) return fallback;
  return command.substring(start).toInt();
}
float commandFloat(const String& command, int start, float fallback) {
  if (command.length() <= start) return fallback;
  return command.substring(start).toFloat();
}
}

void FolkraceRuntime::begin() {
  motorsetup();
#if Memory
  preferences_.begin("folkrace", false);
  speed_ = constrain(preferences_.getInt("speed", speed_), 0, MOTOR_PWM_MAX);
  pid_.kp = preferences_.getFloat("kp", pid_.kp);
  pid_.ki = preferences_.getFloat("ki", pid_.ki);
  pid_.kd = preferences_.getFloat("kd", pid_.kd);
#endif
#if Is_TOF
  tofAvailable_ = tofSetup();
  Serial.println(tofAvailable_ ? "ToF sensors ready." : "ToF setup failed; collision stop is unavailable.");
#endif
  Serial.println("Folkrace runtime ready. Send HELP over serial or BLE.");
  handleCommand("STATUS");
}

void FolkraceRuntime::setState(RobotState state) {
  state_ = state;
  stateStartedAt_ = millis();
  pid_.reset();
  if (state_ != RobotState::RUNNING) stopMotors();
  Serial.printf("state=%s\n", robotStateName(state_));
}

void FolkraceRuntime::save() {
#if Memory
  preferences_.putInt("speed", speed_);
  preferences_.putFloat("kp", pid_.kp);
  preferences_.putFloat("ki", pid_.ki);
  preferences_.putFloat("kd", pid_.kd);
  Serial.println("Settings saved.");
#else
  Serial.println("Memory is disabled in config.h.");
#endif
}

uint16_t FolkraceRuntime::frontDistance() const {
#if Is_TOF
  if (!tofAvailable_) return 0;
  uint16_t closest = 0;
  for (uint8_t i = 0; i < TOF_SENSOR_COUNT; ++i) {
    const TofSensor* sensor = allTofSensors[i];
    if (abs(sensor->angle) <= 45 && sensor->lastReadingMM > 0 &&
        (closest == 0 || sensor->lastReadingMM < closest)) closest = sensor->lastReadingMM;
  }
  return closest;
#else
  return 0;
#endif
}

void FolkraceRuntime::runControl(float dtSeconds) {
#if Is_TOF
  if (tofAvailable_) tofReadAll();
#endif
  const uint16_t front = frontDistance();
  if (front > 0 && front < FRONT_STOP_DISTANCE_MM) {
    Serial.printf("Emergency stop: front distance %u mm\n", front);
    setState(RobotState::IDLE);
    return;
  }

  int correction = 0;
#if Is_TOF
  uint16_t left = 0, right = 0;
  if (tofAvailable_) {
    for (uint8_t i = 0; i < TOF_SENSOR_COUNT; ++i) {
      const TofSensor* sensor = allTofSensors[i];
      if (sensor->angle < 0) left = sensor->lastReadingMM;
      if (sensor->angle > 0) right = sensor->lastReadingMM;
    }
    if (left > 0 && right > 0) correction = pid_.update(static_cast<float>(right) - left, dtSeconds);
  }
#endif

#if TwoMotors
  motorDrive(&motor_left, speed_ - correction);
  motorDrive(&motor_right, speed_ + correction);
#elif tank
  motorDrive(&motor_front_left, speed_ - correction);
  motorDrive(&motor_back_left, speed_ - correction);
  motorDrive(&motor_front_right, speed_ + correction);
  motorDrive(&motor_back_right, speed_ + correction);
#else
  driveAll(speed_);
#endif
}

void FolkraceRuntime::telemetry() const {
  Serial.printf("telemetry state=%s speed=%d kp=%.3f ki=%.3f kd=%.3f front=%u\n",
                robotStateName(state_), speed_, pid_.kp, pid_.ki, pid_.kd, frontDistance());
}

void FolkraceRuntime::update() {
  const uint32_t now = millis();
  if (state_ == RobotState::CALIBRATION) {
    // ToF initialisation was already performed in begin; this state is kept so
    // IMU/other sensor calibration can be added without changing the protocol.
    if (now - stateStartedAt_ > 500) setState(RobotState::READY);
  } else if (state_ == RobotState::COUNTDOWN && now - stateStartedAt_ >= DEFAULT_COUNTDOWN_MS) {
    setState(RobotState::RUNNING);
  } else if (state_ == RobotState::RUNNING && now - lastControlAt_ >= CONTROL_INTERVAL_MS) {
    const float dtSeconds = (lastControlAt_ == 0 ? CONTROL_INTERVAL_MS : now - lastControlAt_) / 1000.0f;
    lastControlAt_ = now;
    runControl(dtSeconds);
  }
  if (now - lastTelemetryAt_ >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryAt_ = now;
    telemetry();
  }
}

void FolkraceRuntime::handleCommand(String command) {
  command.trim();
  command.toUpperCase();
  if (command == "HELP") {
    Serial.println("READY, START, STOP, CALIBRATE, STATUS, SAVE, SPEED <0-255>, KP <value>, KI <value>, KD <value>");
  } else if (command == "READY") {
    setState(RobotState::READY);
  } else if (command == "CALIBRATE") {
    setState(RobotState::CALIBRATION);
  } else if (command == "START") {
    if (state_ == RobotState::READY) setState(RobotState::COUNTDOWN);
    else Serial.println("START is only allowed when READY.");
  } else if (command == "STOP") {
    setState(RobotState::IDLE);
  } else if (command == "STATUS") {
    telemetry();
  } else if (command == "SAVE") {
    save();
  } else if (command.startsWith("SPEED ")) {
    speed_ = constrain(commandInt(command, 6, speed_), 0, MOTOR_PWM_MAX);
    Serial.printf("speed=%d\n", speed_);
  } else if (command.startsWith("KP ")) {
    pid_.kp = commandFloat(command, 3, pid_.kp);
    Serial.printf("kp=%.3f\n", pid_.kp);
  } else if (command.startsWith("KI ")) {
    pid_.ki = commandFloat(command, 3, pid_.ki);
    Serial.printf("ki=%.3f\n", pid_.ki);
  } else if (command.startsWith("KD ")) {
    pid_.kd = commandFloat(command, 3, pid_.kd);
    Serial.printf("kd=%.3f\n", pid_.kd);
  } else if (command.length() > 0) {
    Serial.println("Unknown command. Send HELP.");
  }
}
