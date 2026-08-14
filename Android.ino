#include "DeltaTime.h"

#include "RobotState.h"

#include "libraries.h"

Preferences pref;
static DeltaTime dt;
// PIDC pid(pid);

//Configurate Parameters
float kp, ki, ki;
int maxspeed, minspeed, rspeed, fspeed;
int maxdelta, constrainpid, constraindelta, errorleftdist, errorrightdist, minwalldistFront;
float kleft, kright;

int16_t dist_left, dist_front, dist_right;

unsigned long countdownStartTime = 0;
const unsigned long COUNTDOWN_DURATION = 4500; //milliseconds
RobotState currentState = IDLE;

VL53L8CX_ResultsData results;

void stop() {
  digitalWrite(pin::MA1, LOW);
  digitalWrite(pin::MA2, LOW);
  digitalWrite(pin::MB1, LOW);
  digitalWrite(pin::MB2, LOW);
}

void setup (){
  Serial.begin(115200);

  pref.begin("Folkrace");
  load_state();

  initBLE();

}

void loop(){
  switch (currentState):
    case IDLE:
      stop();

      break;
    case COUNTDOWN:
      if (millis() - countdownStartTime >= COUNTDOWN_DURATION) {
        currentState = RUNNING;
      }
      break;
    case CALIBRATION:
      Serial.printnl("Calibrating Sensors: ");
      #include "Calibration.h"
      #if Is_IMU
      Serial.printnl("Calibrating IMU");
      #endif

      #if Is_TOF
      Serial.printnl("Calibrating Tof distance Sensors");
      #endif

      #if Is_Sharp
      Serial.printnl("Calibrating Sharp distance Sensors");
      #endif

      #if Is_Ultrasonic
      Serial.printnl("Calibrating Ultrasonic distance Sensors");
      #endif
      
      break;
    case RUNNING:
      

}
