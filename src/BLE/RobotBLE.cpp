#include "RobotBLE.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "Commands.h"
#include "notify.h"

#define SERVICE_UUID        "f78982d7-e5b9-43bf-8325-94ea946e903d"
#define CHARACTERISTIC_UUID "33c7a84a-9c35-4887-8f55-6d1c5306f653"

// Defined here, referenced (extern) from notify.cpp so notify() can push
// text out over it without every caller needing to know about BLE.
BLECharacteristic *characteristic = nullptr;

namespace {
  bool clientConnected = false;

  class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *server) override {
      clientConnected = true;
      Serial.println("BLE: client connected");
    }

    void onDisconnect(BLEServer *server) override {
      clientConnected = false;
      Serial.println("BLE: client disconnected, restarting advertising");
      server->getAdvertising()->start();
    }
  };

  // Every write to the characteristic is one line of the Commands.h text
  // protocol, e.g. "set kp 1.5" or "save".
  class CommandCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
      String value = pCharacteristic->getValue();
      uint8_t *data = pCharacteristic->getData();
      size_t data_length = pCharacteristic->getLength();

      char buffer[64];
      size_t copy_length = data_length < sizeof(buffer) - 1 ? data_length : sizeof(buffer) - 1;
      memcpy(buffer, data, copy_length);
      buffer[copy_length] = '\0';

      char *rest = nullptr;
      char *command = strtok_r(buffer, " \r\n", &rest);
      if (command == nullptr) {
        notify("unknown command\n");
        return;
      }
      Commands::handle(command, rest);
    }
  };
}

namespace RobotBLE {

void begin() {
  BLEDevice::init("aqwsedrtvybunmi"); // TODO: give your robot its own name

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);

  characteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ  |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  characteristic->setCallbacks(new CommandCallbacks());

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06); // helps with iPhone connection issues
  advertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE: advertising started");
}

bool connected() {
  return clientConnected;
}

} // namespace RobotBLE
