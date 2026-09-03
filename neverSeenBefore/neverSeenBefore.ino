#include "BleConsole.h"
#include "FolkraceRuntime.h"
#include "RuntimeConfig.h"

FolkraceRuntime robot;
String serialLine;

void receiveCommand(String command) {
  robot.handleCommand(command);
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(200);
  robot.begin();
#if Is_blueTooth
  beginBleConsole(receiveCommand);
#endif
}

void loop() {
  while (Serial.available() > 0) {
    const char input = static_cast<char>(Serial.read());
    if (input == '\n' || input == '\r') {
      if (serialLine.length() > 0) {
        receiveCommand(serialLine);
        serialLine = "";
      }
    } else if (serialLine.length() < 96) {
      serialLine += input;
    } else {
      serialLine = "";
      Serial.println("Command too long.");
    }
  }
  robot.update();
}
