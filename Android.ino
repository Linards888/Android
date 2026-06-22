#include "DeltaTime.h"

#include <Preferences.h>

#include <Wire.h>
#include <VL53L0X.h>
#include <FastIMU.h>
#include "RobotBLE.h"
#include "RobotState.h"

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

void load_state(){
  kp = pref.getFloat("kp");
  ki = pref.getFloat("ki");
  kd = pref.getFloat("kd");
  maxspeed = pref.getInt("maxspeed");
  minspeed = pref.getInt("minspeed");
  maxdelta = pref.getInt("maxdelta");
  rspeed = pref.getInt("rspeed");
  minwalldistFront = pref.getInt("minwalldistFront");
  errorleftdist = pref.getInt("errorleftdist");
  errorrightdist = pref.getInt("errorrightdist");
  constraindelta = pref.getInt("constraindelta");
  constrainpid = pref.getInt("constrainpid");
  kleft = pref.getFloat("kleft");
  kright = pref.getFloat("kright");
  fspeed = pref.getInt("fspeed");
  rspeed = pref.getInt("rspeed");
}

void save_state(){
  pref.putFloat("kp", kp);
  pref.putFloat("ki", ki);
  pref.putFloat("kd", kd);
  pref.putInt("maxspeed", maxspeed);
  pref.putInt("minspeed", minspeed);
  pref.putInt("maxdelta", maxdelta);
  pref.putInt("rspeed", rspeed);
  pref.putInt("minwalldistFront", minwalldistFront);
  pref.putInt("errorleftdist", errorleftdist);
  pref.putInt("errorrightdist", errorrightdist);
  pref.putInt("constraindelta", constraindelta);
  pref.putInt("constrainpid", constrainpid);
  pref.putInt("kleft", kleft);
  pref.putInt("kright", kright);
  pref.putInt("fspeed", fspeed);
  pref.putInt("rspeed", rspeed);
}

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

  //initialise motors
  pinMod(pin::MA1, OUTPUT);
  pinMod(pin::MA2, OUTPUT);
  pinMod(pin::MB1, OUTPUT);
  pinMod(pin::MB2, OUTPUT);
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
      Serial.print("Calibrating IMU: ");
      Serial.print("");
      Serial.printnl("Calibrating Sensors: ");
      //...
      break;
    case RUNNING:

}
