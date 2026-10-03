#pragma once

// BLE GATT server that exposes one characteristic for tuning the robot
// live: write a text command to it (see Commands.h for the protocol -
// "get kp", "set kp 1.5", "save", "state", ...) and it replies over the
// same characteristic's notifications.
//
// Usage:
//   RobotBLE::begin();  // once, in setup(), after ParamRegistry::load()
namespace RobotBLE {
  void begin();

  // True once a central (phone/PC) is connected.
  bool connected();
}
