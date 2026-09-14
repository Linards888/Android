#pragma once
#include <Arduino.h>
#include "../../config.h"

// Nordic UART Service (NUS). TX is notify/read; RX is write.
#define FOLKRACE_NUS_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define FOLKRACE_NUS_RX_UUID      "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define FOLKRACE_NUS_TX_UUID      "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

void bleBegin();
bool bleIsConnected();
bool bleReadCommand(char* destination, size_t destinationSize);
void notify(const char* format, ...);
