#include "DeltaTime.h"

#include <Preferences.h>

#include <Wire.h>
#include <VL53L0X.h>
#include <FastIMU.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID          "deadf33f-a6a6-5155-c0de-a6a6f33f0001"
#define CHARACTERISTIC_UUID   "dec887a8-991b-4501-9409-5b83528be174"

BLEServer *server = nullptr;
BLECharacteristic *characteristic = nullptr;

Preferences pref;
static DeltaTime dt;
// PIDC pid(pid);


float kp;
float ki;
float kd;
int maxspeed;
int minspeed;
int maxdelta;
int rspeed;
int minwalldistFront;
int errorleftdist;
int errorrightdist;
int constraindelta;
int constrainpid;
float kleft;
float kright;
int fspeed;

int16_t dist_left_side;
int16_t dist_left;
int16_t dist_front;
int16_t dist_right;
int16_t dist_right_side;
int16_t dist_right_side1;

VL53L8CX_ResultsData results;

void load_state(){
  kp = pref.getFloat("kp");
  ki = pref.getFloat("ki");
  kd = pref.getFloat("kd");
  maxspeed = pref.getInt("maxspeed");
  minspeed = pref.getInt("minspeed");
  maxdelta = pref.getInt("maxdelta");
  rspeed = pref.getInt("rspeed");
  minwalldistFront = pref.getInt("minwalldist");
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

void setup (){
  Serial.begin(115200);

  pref.begin("Folkrace");
  load_state();

  BLEDevice::init("Folkrace");
  server = BLEDevice::createServer();

  BLEService *service = server->createService(SERVICE_UUID);

  characteristic = static->createCaracteristic(
          CHARACTERISTIC_UUID,
          BLECharacteristic::PROPERTY_READ   |
          BLECharacteristic::PROPERTY_WRITE  |
          BLECharacteristic::PROPERTY_NOTIFY
        );
  characteristic->setCallbacks(new MyCallbacks());

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->setServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPrefered(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE start");

  //initialise motors
  pinMod(pin::MA1, OUTPUT);
  pinMod(pin::MA2, OUTPUT);
  pinMod(pin::MB1, OUTPUT);
  pinMod(pin::MB2, OUTPUT);
}

void loop(){
  if (state == 1){
    
  }
}
