#include "DeltaTime.h"
#include "RobotState.h"
#include "Libraries.h"

Preferences pref;
static DeltaTime dt;
// PIDC pid(pid);

int16_t dist_left, dist_front, dist_right;

unsigned long countdownStartTime = 0;
const unsigned long COUNTDOWN_DURATION = 5000; //milliseconds
RobotState currentState = IDLE;

void calibration_run() {
  Serial.println("calibration not implemented yet");
  currentState = IDLE;
}

void setup() {
  Serial.begin(115200);

  pref.begin("Folkrace");
  ParamRegistry::load(pref);

  Motorsetup();

#if Is_blueTooth
  RobotBLE::begin();
#endif

  // ---- Manual parameter overrides ----
  // Every tunable in src/Params/Params.h is a plain global
  // Anything you set here wins last.
  // kp = 1.2f;
}

void loop(){
#if Is_blueTooth
  Commands::pollSerial();
#endif

  switch (currentState){
    case IDLE:
      stop();
    break;
    case CALIBRATION:
      Serial.println("Calibrating Sensors: ");
      calibration_run();
    break;
    case READY:
    break;
    case FORWARD:
    break;
    case REVERSE:
    break;
    case COUNTDOWN:
      if (millis() - countdownStartTime >= COUNTDOWN_DURATION) {
        currentState = RUNNING;
      }
    break;
    case RUNNING:
    break;

  }

}
