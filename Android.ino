#include "DeltaTime.h"

#include "RobotState.h"

#include "libraries.h"

#include "RobotBLE.h"

#include "Calibration.h"


Preferences pref;
static DeltaTime dt;
// PIDC pid(pid);

//Configurate Parameters
float kp, ki, kd;
int maxspeed, minspeed, rspeed, fspeed;
int maxdelta, constrainpid, constraindelta, errorleftdist, errorrightdist, minwalldistFront;
float kleft, kright;

int16_t dist_left, dist_front, dist_right;

unsigned long countdownStartTime = 0;
const unsigned long COUNTDOWN_DURATION = 4500; //milliseconds
RobotState currentState = IDLE;

void setup (){
  Serial.begin(115200);

  pref.begin("Folkrace");
  load_state();

  initBLE();

}

void loop(){
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
    case BACKWARDS:
    break;
    case COUNTDOWN:
      if (millis() - countdownStartTime >= COUNTDOWN_DURATION) {
        currentState = RUNNING;
      }
    break;
    case RUNNING:
      //edit main code and logic here:
      
    break;

  }

}
