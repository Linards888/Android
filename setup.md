# Folkrace firmware setup

This guide prepares a development computer and configures the firmware for a
specific Folkrace robot. Read through the hardware configuration before
connecting power to motors.

## 1. Get the project

Open PowerShell (or a terminal), go to the folder where you keep projects, and
clone the repository:

```powershell
git clone https://github.com/Linards888/Android.git
cd Android
```

The sketch entry point is `Android.ino`. Project code is grouped under `src/`
and the robot-specific settings are in `config/config.h`.

## 2. Install the ESP32 development tools

1. Install [Arduino IDE 2](https://www.arduino.cc/en/software).
2. Open **File > Preferences** and add this URL to **Additional Boards Manager
   URLs**:

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```

3. Open **Tools > Board > Boards Manager**, search for **esp32**, and install
   **esp32 by Espressif Systems**.
4. Connect the ESP32 by USB. Install its USB-to-serial driver if no serial port
   appears in **Tools > Port**.

Use an ESP32 board, not an Arduino Uno/Nano: this firmware uses ESP32 features
such as Preferences and Bluetooth Low Energy.

## 3. Install libraries for the hardware you enable

In Arduino IDE, open **Tools > Manage Libraries** and install only the libraries
needed by the features you turn on in `config/config.h`:

| Feature | Library |
| --- | --- |
| IMU (`Is_IMU`) | FastIMU |
| VL53L0X distance sensors (`Is_TOF`) | VL53L0X by Pololu |
| Servo (`Is_servo`) | kkads_servo |
| BLE (`Is_blueTooth`) | Provided by the installed ESP32 board package |
| Persistent settings (`Memory`) | Provided by the installed ESP32 board package |

`Wire`, `Arduino`, and `Preferences` are supplied by the Arduino/ESP32
framework and do not need separate installation.

## 4. Configure the robot

Open [`config/config.h`](config/config.h) before building. Change the `0` and
`1` values to describe the robot you actually built.

1. Leave `Is_Esp32` set to `1` and `Is_Arduino` set to `0`.
2. Enable the sensors that are physically fitted. For example, set `Is_TOF` to
   `1` only when VL53L0X sensors are connected.
3. Select exactly one drive type: `OneMotor`, `TwoMotors`, or `tank`. Set
   `Is_servo` to `1` as well if the steering servo is installed.
4. Enable optional features such as `Is_blueTooth`, `Is_IMU`, `Memory`, and
   `spaceControl` only after their hardware and libraries are ready.
5. Update the sensor lists, motor pins, `SERVO_PIN`, and IMU address so they
   match the wiring. The pin values in the file are examples, not universal
   ESP32 wiring assignments.

Never enable a motor configuration until its pin assignments and motor-driver
wiring have been checked. Keep the wheels off the ground for the first test.

## 5. Open and build the sketch

Open `Android.ino` in Arduino IDE. Select the exact ESP32 board under
**Tools > Board** and select the USB serial port under **Tools > Port**. Start
with the default ESP32 upload settings unless the board manufacturer specifies
otherwise.

The source files are stored in subfolders under `src/`. Configure your editor's
include paths for these folders when it reports missing headers:

```text
config
src/Utils
src/drive
src/Sensors
src/IMU
src/BLE
src/Calibrations
src/spaceAverenes
```

The repository does not currently include a PlatformIO configuration, so use
the Arduino toolchain only after ensuring the build environment can compile
those source folders. If your Arduino IDE installation does not compile sketch
source recursively, use an Arduino-compatible project configuration that adds
the folders above to the build, or place the required sources alongside the
sketch for local testing. Avoid committing copied source files.

## 6. Upload and check the first boot

1. Click **Verify** to compile.
2. Disconnect motor power, then click **Upload**.
3. Open **Tools > Serial Monitor** and set it to **115200 baud**.
4. Confirm the board starts without reset loops or missing-device errors.
5. Reconnect motor power only after sensor readings, direction, and stop
   behaviour have been verified.

When Bluetooth is enabled, the firmware advertises itself as `Folkrace`.

## Troubleshooting

- **No serial port:** use a data-capable USB cable and install the board's USB
  driver.
- **`No board selected` or `Multiple board configurations selected`:** correct
  `Is_Arduino` and `Is_Esp32` in `config/config.h`.
- **Missing library error:** install the library named in the compile error, or
  disable the related feature flag.
- **No motor configuration selected:** enable exactly one of `OneMotor`,
  `TwoMotors`, and `tank`.
- **ESP32 will not upload:** hold the board's **BOOT** button while upload
  begins, then release it when the connection starts.
