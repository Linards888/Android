/*
 * Folkrace main program
 * =====================
 * Keep your robot-specific code in the USER LOGIC section below.
 * config.h is for wiring and fitted hardware; this file is for behaviour.
 */

#include "config.h"
#include "src/Utils/RobotState.h"
#include "src/drive/Drive.h"
#include "src/BLE/RobotBLE.h"
#include "src/IMU/IMU_logic.h"

RobotState currentState = IDLE;
IMUReading imuReading = {};

// ===========================================================================
// USER LOGIC
// ===========================================================================
// These values are updated by DRIVE/TANK phone commands.  You can use them
// directly, or replace runRobot() with wall-following/PID/autonomous logic.
int driveThrottle = 0;      // -FOLKRACE_MAX_POWER to +FOLKRACE_MAX_POWER
int driveSteering = 0;      // -FOLKRACE_MAX_POWER to +FOLKRACE_MAX_POWER
int driveLeft = 0;          // only used by TANK mode
int driveRight = 0;         // only used by TANK mode
bool useTankDrive = false;

void setupRobot() {
  // Put one-time setup for your sensors, servo, PID, etc. here.
}

void readSensors(float dtSeconds) {
  // Read distance sensors/encoders here.
  // Example: frontDistance = tof_read(&tof_front);
  (void)dtSeconds;
}

void runRobot(float dtSeconds) {
  // This is the main driving logic. Replace this body with your own PID,
  // wall-following or race code. The default is direct phone control.
  (void)dtSeconds;

  if (useTankDrive) {
    driveTank(driveLeft, driveRight);
  } else {
    driveArcade(driveThrottle, driveSteering);
  }
}

void onIMUReading(const IMUReading& reading) {
  // Runs whenever FastIMU provides a new sample.
  // reading.ax/ay/az are acceleration; reading.gx/gy/gz are gyro values.
  (void)reading;
}

bool handleCustomCommand(const char* command, char* arguments) {
  // Add your own BLE/USB commands here. Return true when you handled one.
  //
  // Example:
  // if (!strcmp(command, "LED")) { digitalWrite(LED_BUILTIN, HIGH); return true; }
  (void)command;
  (void)arguments;
  return false;
}

// ===========================================================================
// CONTROL FRAMEWORK
// You normally do not need to edit below this line.
// ===========================================================================

struct ManualCommand {
  unsigned long lastReceivedMs = 0;
  unsigned long countdownStartedMs = 0;
  unsigned long countdownDurationMs = 0;
} manualCommand;

static const char* stateName(RobotState state) {
  switch (state) {
    case IDLE:      return "IDLE";
    case READY:     return "READY";
    case COUNTDOWN: return "COUNTDOWN";
    case RUNNING:   return "RUNNING";
  }
  return "UNKNOWN";
}

static void stopRobot() {
  driveThrottle = driveSteering = 0;
  driveLeft = driveRight = 0;
  useTankDrive = false;
  stopMotors();
}

static bool parseTwoIntegers(char* text, int& first, int& second) {
  char* end = nullptr;
  first = strtol(text, &end, 10);
  if (end == text) return false;
  second = strtol(end, &end, 10);
  return true;
}

static void printHelp() {
  notify("Commands:\n"
         "  READY                 arm the car\n"
         "  START [milliseconds]  start now or after a countdown\n"
         "  STOP                  immediate motor stop\n"
         "  DRIVE <throttle> <steering>  -255..255\n"
         "  TANK <left> <right>          -255..255\n"
         "  STATUS, IMU, HELP\n");
}

static void handleCommand(char* text) {
  while (*text == ' ') ++text;
  if (!*text) return;

  char* arguments = strchr(text, ' ');
  if (arguments) *arguments++ = '\0';
  else arguments = text + strlen(text);

  for (char* letter = text; *letter; ++letter) *letter = toupper(*letter);

  if (!strcmp(text, "HELP")) {
    printHelp();
  } else if (!strcmp(text, "STATUS")) {
    notify("state=%s ble=%d imu=%d drive=%d,%d\n", stateName(currentState),
           bleIsConnected(), imuIsReady(), driveThrottle, driveSteering);
  } else if (!strcmp(text, "READY")) {
    currentState = READY;
    stopRobot();
    notify("ready\n");
  } else if (!strcmp(text, "STOP")) {
    currentState = IDLE;
    stopRobot();
    notify("stopped\n");
  } else if (!strcmp(text, "START")) {
    if (currentState != READY) {
      notify("ERR: send READY first\n");
      return;
    }
    const long delayMs = *arguments ? constrain(strtol(arguments, nullptr, 10), 0L, 10000L) : 0;
    manualCommand.countdownStartedMs = millis();
    manualCommand.countdownDurationMs = delayMs;
    manualCommand.lastReceivedMs = millis();
    currentState = delayMs ? COUNTDOWN : RUNNING;
    if (delayMs) notify("starting in %ld ms\n", delayMs);
    else notify("running\n");
  } else if (!strcmp(text, "DRIVE")) {
    int throttle, steering;
    if (!parseTwoIntegers(arguments, throttle, steering)) {
      notify("ERR: DRIVE <throttle> <steering>\n");
      return;
    }
    if (currentState == IDLE) {
      notify("ERR: send READY then START first\n");
      return;
    }
    driveThrottle = constrain(throttle, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
    driveSteering = constrain(steering, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
    useTankDrive = false;
    manualCommand.lastReceivedMs = millis();
    notify("drive %d %d\n", driveThrottle, driveSteering);
  } else if (!strcmp(text, "TANK")) {
    if (!TwoMotors) {
      notify("ERR: TANK requires TwoMotors in config.h\n");
      return;
    }
    int left, right;
    if (!parseTwoIntegers(arguments, left, right)) {
      notify("ERR: TANK <left> <right>\n");
      return;
    }
    if (currentState == IDLE) {
      notify("ERR: send READY then START first\n");
      return;
    }
    driveLeft = constrain(left, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
    driveRight = constrain(right, -FOLKRACE_MAX_POWER, FOLKRACE_MAX_POWER);
    useTankDrive = true;
    manualCommand.lastReceivedMs = millis();
    notify("tank %d %d\n", driveLeft, driveRight);
  } else if (!strcmp(text, "IMU")) {
    if (!imuIsReady()) {
      notify("IMU disabled or not found\n");
    } else {
      notify("accel=%.3f,%.3f,%.3f gyro=%.3f,%.3f,%.3f\n",
             imuReading.ax, imuReading.ay, imuReading.az,
             imuReading.gx, imuReading.gy, imuReading.gz);
    }
  } else if (!handleCustomCommand(text, arguments)) {
    notify("ERR: unknown command. Send HELP\n");
  }
}

static void readSerialCommands() {
#if FOLKRACE_ENABLE_SERIAL
  static char buffer[96];
  static size_t used = 0;

  while (Serial.available()) {
    const char character = Serial.read();
    if (character == '\n' || character == '\r') {
      if (used) {
        buffer[used] = '\0';
        handleCommand(buffer);
        used = 0;
      }
    } else if (used < sizeof(buffer) - 1) {
      buffer[used++] = character;
    }
  }
#endif
}

void setup() {
  Serial.begin(115200);
  driveBegin();

#if Is_IMU
  imuBegin();
#endif
#if Is_blueTooth
  bleBegin();
#endif

  setupRobot();
  notify("Folkrace ready. Send HELP\n");
}

void loop() {
  static unsigned long previousLoopMs = millis();
  const unsigned long now = millis();
  if (now - previousLoopMs < FOLKRACE_LOOP_PERIOD_MS) return;

  const float dtSeconds = (now - previousLoopMs) / 1000.0f;
  previousLoopMs = now;

  readSerialCommands();
  char command[96];
  while (bleReadCommand(command, sizeof(command))) handleCommand(command);

  if (imuUpdate(imuReading)) onIMUReading(imuReading);
  readSensors(dtSeconds);

  if (currentState == COUNTDOWN) {
    if (now - manualCommand.countdownStartedMs >= manualCommand.countdownDurationMs) {
      currentState = RUNNING;
      manualCommand.lastReceivedMs = now;
      notify("running\n");
    }
    return;
  }

  if (currentState != RUNNING) {
    stopMotors();
    return;
  }

  if (now - manualCommand.lastReceivedMs > FOLKRACE_COMMAND_TIMEOUT_MS) {
    stopRobot();
    return;
  }

  runRobot(dtSeconds);
}
