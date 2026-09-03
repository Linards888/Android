#include "BleConsole.h"

#if Is_blueTooth
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

namespace {
constexpr char SERVICE_UUID[] = "deadf33f-a6a6-5155-c0de-a6a6f33f0001";
constexpr char CHARACTERISTIC_UUID[] = "dec887a8-991b-4501-9409-5b83528be174";
void (*onCommand)(String) = nullptr;

class CommandCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    if (onCommand == nullptr) return;
    // .c_str() keeps this compatible with ESP32 BLE core versions whose
    // getValue() returns either std::string or Arduino String.
    const String value = characteristic->getValue().c_str();
    if (value.length() > 0) onCommand(value);
  }
};
}

void beginBleConsole(void (*commandHandler)(String)) {
  onCommand = commandHandler;
  BLEDevice::init("Folkrace");
  BLEServer* server = BLEDevice::createServer();
  BLEService* service = server->createService(SERVICE_UUID);
  BLECharacteristic* characteristic = service->createCharacteristic(
      CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  characteristic->setCallbacks(new CommandCallbacks());
  service->start();
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();
  Serial.println("BLE command console ready.");
}
#endif
