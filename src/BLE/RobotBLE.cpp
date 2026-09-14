#include "RobotBLE.h"

#if FOLKRACE_ENABLE_BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

static BLECharacteristic* txCharacteristic = nullptr;
static QueueHandle_t commandQueue = nullptr;
static volatile bool connected = false;
static constexpr size_t COMMAND_SIZE = 96;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) override { connected = true; }
  void onDisconnect(BLEServer*) override {
    connected = false;
    BLEDevice::startAdvertising();
  }
};

class RxCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    std::string received = characteristic->getValue();
    if (received.empty() || commandQueue == nullptr) return;
    char command[COMMAND_SIZE] = {};
    const size_t length = min(received.size(), sizeof(command) - 1);
    memcpy(command, received.data(), length);
    size_t used = strlen(command);
    while (used && (command[used - 1] == '\r' || command[used - 1] == '\n'))
      command[--used] = '\0';
    xQueueSend(commandQueue, command, 0); // Never block the BLE task.
  }
};

void bleBegin() {
  commandQueue = xQueueCreate(8, COMMAND_SIZE);
  BLEDevice::init(FOLKRACE_BLE_NAME);
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  BLEService* service = server->createService(FOLKRACE_NUS_SERVICE_UUID);
  txCharacteristic = service->createCharacteristic(FOLKRACE_NUS_TX_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  txCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic* rxCharacteristic = service->createCharacteristic(FOLKRACE_NUS_RX_UUID,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  rxCharacteristic->setCallbacks(new RxCallbacks());
  service->start();
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(FOLKRACE_NUS_SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("BLE UART ready");
}

bool bleIsConnected() { return connected; }
bool bleReadCommand(char* destination, size_t destinationSize) {
  if (!commandQueue || !destination || destinationSize < 2) return false;
  char received[COMMAND_SIZE];
  if (xQueueReceive(commandQueue, received, 0) != pdTRUE) return false;
  strncpy(destination, received, destinationSize - 1);
  destination[destinationSize - 1] = '\0';
  return true;
}

void notify(const char* format, ...) {
  char message[192];
  va_list args;
  va_start(args, format);
  vsnprintf(message, sizeof(message), format, args);
  va_end(args);
  Serial.print(message);
  if (connected && txCharacteristic) {
    txCharacteristic->setValue((uint8_t*)message, strlen(message));
    txCharacteristic->notify();
  }
}
#else
void bleBegin() {}
bool bleIsConnected() { return false; }
bool bleReadCommand(char*, size_t) { return false; }
void notify(const char* format, ...) {
  va_list args; va_start(args, format); Serial.vprintf(format, args); va_end(args);
}
#endif
