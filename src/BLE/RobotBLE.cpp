#include "RobotBLE.h"

#if Is_blueTooth
#include <stdarg.h>
#include "commands.h"

BLEServer *server = nullptr;
BLECharacteristic *characteristic = nullptr;

void initBLE() {
  BLEDevice::init(BLE_DEVICE_NAME);
  server = BLEDevice::createServer();
  server->setCallbacks(new MyServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);

  characteristic = service->createCharacteristic(
                     CHARACTERISTIC_UUID,
                     BLECharacteristic::PROPERTY_READ   |
                     BLECharacteristic::PROPERTY_WRITE  |
                     BLECharacteristic::PROPERTY_NOTIFY
                   );

  characteristic->setCallbacks(new MyCallbacks());
  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();

  Serial.println("BLE started successfully");
}

void notify(const char* fmt, ...) {
  if (!characteristic) return;

  char buffer[128];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  characteristic->setValue((uint8_t*)buffer, strlen(buffer));
  characteristic->notify();
}

void MyServerCallbacks::onConnect(BLEServer* pServer) {
  Serial.println("BLE client connected");
}

void MyServerCallbacks::onDisconnect(BLEServer* pServer) {
  Serial.println("BLE client disconnected, restarting advertising");
  // The ESP32 BLE stack stops advertising on disconnect unless told
  // otherwise, which would otherwise make the robot unreachable again.
  BLEDevice::startAdvertising();
}

void MyCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
  String value = pCharacteristic->getValue();
  if (value.length() == 0) return;

  Serial.print("BLE RX: ");
  Serial.println(value);

  static char buffer[128];
  size_t len = value.length();
  if (len > sizeof(buffer) - 1) len = sizeof(buffer) - 1;
  memcpy(buffer, value.c_str(), len);
  buffer[len] = '\0';

  char* rest = buffer;
  char* command = strtok_r(rest, " \r\n", &rest);
  if (command == nullptr) return;

  handle_command(command, rest);
}

#endif
