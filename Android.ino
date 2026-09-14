#include "config.h"
#include "src/Utils/RobotState.h"
#include "src/drive/Drive.h"
#include "src/BLE/RobotBLE.h"
#include "src/IMU/IMU_logic.h"

RobotState currentState = IDLE;
static IMUReading imuReading = {};
static int throttleCommand = 0;
static int steeringCommand = 0;
static bool tankMode = false;
static int tankLeftCommand = 0;
static int tankRightCommand = 0;
static unsigned long lastDriveCommandMs = 0;
static unsigned long countdownStartedMs = 0;
static unsigned long countdownDurationMs = 0;

// ---------------------------------------------------------------------------
// Put your car-specific code in these hooks.  The framework never changes
// them, so this sketch is the place to write wall following, PID and sensors.
// ---------------------------------------------------------------------------
void onRobotReady() {}

void onRobotRunning(float dtSeconds) {
  // Default: phone joystick control. Replace or augment this with your PID.
  // Example: driveArcade(pidOutput, steeringCorrection);
  (void)dtSeconds;
  if (tankMode) driveTank(tankLeftCommand, tankRightCommand);
  else driveArcade(throttleCommand, steeringCommand);
}

void onIMUUpdated(const IMUReading& reading) {
  // Example: use reading.gz for turn-rate damping or reading.ax for acceleration.
  (void)reading;
}

static const char* stateName(RobotState state) {
  switch (state) {
    case IDLE: return "IDLE";
    case READY: return "READY";
    case COUNTDOWN: return "COUNTDOWN";
    case RUNNING: return "RUNNING";
  }
  return "UNKNOWN";
}

static void printHelp() {
  notify("Commands:\\n"
         "  READY                 arm the car (motors remain stopped)\\n"
         "  START [milliseconds]  begin driving after optional countdown\\n"
         "  STOP                  immediate motor stop\\n"
         "  DRIVE <throttle> <steering>  values -255..255\\n"
         "  TANK <left> <right>          values -255..255\\n"
         "  STATUS, IMU, HELP\\n");
}

static bool parseTwoIntegers(char* arguments, int& first, int& second) {
  char* end = nullptr;
  first = strtol(arguments, &end, 10);
  if (end == arguments) return false;
  second = strtol(end, &end, 10);
  return true;
}

static void handleCommand(char* line) {
  while (*line == ' ') ++line;
  for (char* p = line; *p; ++p) *p = toupper(*p);
  char* arguments = strchr(line, ' ');
  if (arguments) *arguments++ = '\0'; else arguments = line + strlen(line);

  if (!strcmp(line, "HELP")) { printHelp(); return; }
  if (!strcmp(line, "STATUS")) {
    notify("state=%s ble=%d imu=%d throttle=%d steering=%d\\n", stateName(currentState),
           bleIsConnected(), imuIsReady(), throttleCommand, steeringCommand);
    return;
  }
  if (!strcmp(line, "READY")) {
    currentState = READY;
    throttleCommand = steeringCommand = 0; tankMode = false;
    stopMotors();
    onRobotReady();
    notify("ready\\n");
    return;
  }
  if (!strcmp(line, "STOP")) {
    currentState = IDLE;
    throttleCommand = steeringCommand = 0; tankMode = false;
    stopMotors();
    notify("stopped\\n");
    return;
  }
  if (!strcmp(line, "START")) {
    if (currentState != READY) { notify("ERR: send READY first\\n"); return; }
    const long delayMs = *arguments ? constrain(strtol(arguments, nullptr, 10), 0L, 10000L) : 0;
    countdownStartedMs = millis();
    countdownDurationMs = delayMs;
    lastDriveCommandMs = millis();
    currentState = delayMs ? COUNTDOWN : RUNNING;
    if (delayMs) notify("starting in %ld ms\\n", delayMs);
    else notify("running\\n");
    return;
  }
  if (!strcmp(line, "DRIVE")) {
    int throttle, steering;
    if (!parseTwoIntegers(arguments, throttle, steering)) { notify("ERR: DRIVE <throttle> <steering>\\n"); return; }
    if (currentState == IDLE) { notify("ERR: send READY then START first\\n"); return; }
    throttleCommand = constrain(throttle, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
    steeringCommand = constrain(steering, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
    tankMode = false;
    lastDriveCommandMs = millis();
    notify("drive %d %d\\n", throttleCommand, steeringCommand);
    return;
  }
  if (!strcmp(line, "TANK")) {
    int left, right;
    if (!parseTwoIntegers(arguments, left, right)) { notify("ERR: TANK <left> <right>\\n"); return; }
    if (currentState == IDLE) { notify("ERR: send READY then START first\\n"); return; }
    throttleCommand = steeringCommand = 0;
    tankLeftCommand = constrain(left, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
    tankRightCommand = constrain(right, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
    tankMode = true;
    lastDriveCommandMs = millis();
    notify("tank %d %d\\n", left, right);
    return;
  }
  if (!strcmp(line, "IMU")) {
    if (!imuIsReady()) { notify("IMU disabled or not found\\n"); return; }
    notify("accel=%.3f,%.3f,%.3f gyro=%.3f,%.3f,%.3f\\n", imuReading.ax, imuReading.ay, imuReading.az,
           imuReading.gx, imuReading.gy, imuReading.gz);
    return;
  }
  notify("ERR: unknown command. Send HELP\\n");
}

static void readUsbCommands() {
#if FOLKRACE_ENABLE_SERIAL
  static char buffer[96]; static size_t used = 0;
  while (Serial.available()) {
    const char character = Serial.read();
    if (character == '\n' || character == '\r') {
      if (used) { buffer[used] = '\0'; handleCommand(buffer); used = 0; }
    } else if (used < sizeof(buffer) - 1) buffer[used++] = character;
  }
#endif
}

void setup() {
  Serial.begin(115200);
  driveBegin();
  if (FOLKRACE_ENABLE_IMU) imuBegin();
  if (FOLKRACE_ENABLE_BLE) bleBegin();
  notify("Folkrace ready. Send HELP\\n");
}

void loop() {
  static unsigned long previousLoopMs = millis();
  const unsigned long now = millis();
  if (now - previousLoopMs < FOLKRACE_LOOP_PERIOD_MS) return;
  const float dtSeconds = (now - previousLoopMs) / 1000.0f;
  previousLoopMs = now;

  readUsbCommands();
  char command[96];
  while (bleReadCommand(command, sizeof(command))) handleCommand(command);

  if (imuUpdate(imuReading)) onIMUUpdated(imuReading);

  if (currentState == COUNTDOWN) {
    // Countdown duration is sent with START. This starter uses a fixed five seconds.
    if (now - countdownStartedMs >= countdownDurationMs) {
      currentState = RUNNING;
      lastDriveCommandMs = now; // allow a preloaded control value for one watchdog period
      notify("running\\n");
    }
    return;
  }
  if (currentState != RUNNING) { stopMotors(); return; }
  if (now - lastDriveCommandMs > FOLKRACE_COMMAND_TIMEOUT_MS) {
    throttleCommand = steeringCommand = 0;
    stopMotors();
    return;
  }
  onRobotRunning(dtSeconds);
}
