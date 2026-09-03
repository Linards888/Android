# Completed firmware based on the original project

This folder completes the architecture already present in the repository instead of replacing it with a separate design. It directly reuses the feature switches, `MOTOR_LIST`, and `TOF_SENSOR_LIST` from `../config.h`.

Open `neverSeenBefore.ino` in Arduino IDE. Before compiling, edit the existing `config.h` and select exactly one motor configuration: `OneMotor`, `TwoMotors`, or `tank`. The repository currently has all three disabled, so no motor firmware can compile until you make that hardware choice.

The completed runtime implements the original intended pieces:

- Motor-list based H-bridge control with PWM and a safe stop state.
- ToF sensor startup with XSHUT address assignment and cached readings.
- PID control from left/right ToF readings when both are configured.
- A front-distance emergency stop.
- The original robot lifecycle: IDLE, READY, CALIBRATION, COUNTDOWN, RUNNING.
- Serial commands and optional BLE commands: `READY`, `START`, `STOP`, `CALIBRATE`, `STATUS`, `SAVE`, `SPEED 120`, `KP 1.2`, `KI 0.1`, `KD 0.02`.
- ESP32 Preferences persistence when `Memory` is enabled.

If `Is_TOF` is enabled, install **VL53L0X by Pololu**. BLE uses the ESP32 board package. The code never starts the motors on boot; it requires `READY` then `START`.
