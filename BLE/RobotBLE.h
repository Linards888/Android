#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID          "deadf33f-a6a6-5155-c0de-a6a6f33f0001"
#define CHARACTERISTIC_UUID   "dec887a8-991b-4501-9409-5b83528be174"

extern BLEServer *server;
extern BLECharacteristic *characteristic;

void initBLE();
void notify(const char* fmt, ...);

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override;
  void onDisconnect(BLEServer* pServer) override;
};

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override;
};