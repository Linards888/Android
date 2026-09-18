#include "RobotBLE.h"

#if FEATURE_BLUETOOTH

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <string.h>

#include "commands.h"
#include "../Utils/Notify.h"

#define SERVICE_UUID        "deadf33f-a6a6-5155-c0de-a6a6f33f0001"
#define CHARACTERISTIC_UUID "dec887a8-991b-4501-9409-5b83528be174"

static BLEServer* server = nullptr;
static BLECharacteristic* characteristic = nullptr;

class RobotBLEServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* s) override {
        Serial.println("BLE: device connected");
    }
    void onDisconnect(BLEServer* s) override {
        Serial.println("BLE: device disconnected, restarting advertising");
        s->getAdvertising()->start();
    }
};

class RobotBLEWriteCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* c) override {
        uint8_t* data = c->getData();
        size_t data_length = c->getLength();
        if (data == nullptr || data_length == 0) {
            notify("unknown command\n");
            return;
        }

        char buffer[64];
        size_t copy_length = data_length < sizeof(buffer) - 1 ? data_length : sizeof(buffer) - 1;
        memcpy(buffer, data, copy_length);
        buffer[copy_length] = '\0';

        char* save = nullptr;
        char* command = strtok_r(buffer, " \r\n", &save);
        if (command == nullptr) {
            notify("unknown command\n");
            return;
        }
        handle_command(command, save);
    }
};

static void ble_notify_sink(const char* message) {
    if (characteristic == nullptr) return;
    characteristic->setValue(reinterpret_cast<uint8_t*>(const_cast<char*>(message)), strlen(message));
    characteristic->notify();
}

void ble_init() {
    BLEDevice::init(ROBOT_NAME);

    server = BLEDevice::createServer();
    server->setCallbacks(new RobotBLEServerCallbacks());

    BLEService* service = server->createService(SERVICE_UUID);

    characteristic = service->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ  |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    characteristic->setCallbacks(new RobotBLEWriteCallbacks());

    service->start();

    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06); // helps with some iPhone connection issues
    advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    notify_set_sink(ble_notify_sink);

    Serial.println("BLE started");
}

#endif
