#include "RobotBLE.h"

#include "RobotState.h"

extern float kp, ki, kd;
extern int maxspeed, minspeed, fspeed, rspeed;
extern void save_state(); // Allows BLE to trigger a save to Preferences

BLEServer *server = nullptr;
BLECharacteristic *characteristic = nullptr;

void initBLE() {
  BLEDevice::init("Folkrace");
  server = BLEDevice::createServer();

  BLEService *service = server->createService(SERVICE_UUID);

  // Fixed a minor syntax typo from original draft: (static->createCharacteristic)
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
  advertising->setMinPreferred(0x06);  // Corrected property name typo
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

  characteristic->setValue(buffer);
  characteristic->notify();
}

// Handle incoming BLE data (e.g., tuning parameters mid-run)
void MyCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
  String value = pCharacteristic->getValue();

  if (value.length() > 0) {
    Serial.print("Received BLE Data: ");
    Serial.println(value.c_str());
    
    // Example: If your app sends "START", update the state machine
    if (value == "START") {
      currentState = CALIBRATION;
    }
    
    // Parse incoming tuning payloads here if necessary
    // Example parsed result: kp = newValue; save_state();
  }
}