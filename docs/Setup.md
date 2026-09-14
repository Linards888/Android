# Folkrace Setup

## 1. Get the project

Open cmd, go to folder where you keep your projects, and clone the repo:

```powershell
git clone https://github.com/Linards888/Android
cd Android
```

The main sketch is `Android.ino`. Project code is grouped under `src/`(hopefully there is no need to go there). and the robot configuration is `config.h`.

## Install ESP32 development tools

### Arduino IDE

1. Install[Arduino IDE](https://github.com/Linards888/Android).
2. Open **File > Preferences** and add this URL to **Additional Boards Manager
   URLs**:

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Open **Tools > board > Boards Manager**, search for **esp32**, and install **esp32 by Espressif Systems**.
4. Connect the EPS32 by USB. Install its USB-to-serial driver if no serial port appiers in **Tools > Ports**.

> [!CAUTION]
> Use an ESP32 board, not an Arduino Uno/Nano: this firmware uses ESP32 features such as Preferences and Bluetooth Low Energy.

### Arduino CLI
> [!NOTE]
> Coming Soon!

### Arduino Community Edition - VScode
> [!NOTE]
> Coming Soon!

## 3. Install libraries


In Arduino IDE, open **Tools > Manage Libraries** and install only the libraries
needed by the features you turn on in `config.h`:

| Feature | Library |
| --- | --- |
| IMU (`Is_IMU`) | FastIMU |
| VL53L0X distance sensors (`Is_TOF`) | VL53L0X by Pololu |
| Servo (`Is_servo`) | kkads_servo |
| BLE (`Is_blueTooth`) | Provided by the installed ESP32 board package |

`Wire`, `Arduino`, and `Preferences` are supplied by the Arduino/ESP32
framework and do not need separate installation.

## 4. Configure your robot.

> [!NOTE]
> 1. Go to [`config.h`](../config.h) before building. Set the motor-driver pins, direction flags, I2C pins/address, and enabled features to describe the robot you actually built.
> 2. Select your chosen Board and serial Port.
> 3. Go to `Tools` and set theese settings:
>  USB CDC On Boot: `Enabled`
>  Erase All Flash Before Sketch Upload: `Disabled`
>  Upload Speed: (preferably `115200`) as high as possible, for fastest upload
> 4. Set boud rate to **115200**.

> [!WARNING]
> 1. Leave `Is_Esp32` set to `1` and `Is_Arduino` set to `0`.
> 1. Leave `Memory` set to `1`. 
> 2. Enable sensors that are physically fitted. For example, set `Is_TOF` to `1` only when VL53L0X sensors are connected.
> 3. Select exactly one drive type: `OneMotor`, `TwoMotors`, or `tank`. Set `Is_servo` to `1` as well if the steering servo is installed.
> 4. Enable optional features such as `Is_blueTooth`, `Is_IMU`
> 5. Update the sensor lists, motor pins.

## 6. Upload the sketch

1. Connect Microcontroller to PC and select **Serial Port**.
2. CLick **Upload**, and wait for the code to upload.
3. If there are errors, check `trubleshooting.md`.

> [!NOTE]
> When Bluetooth is enabled, the firmware advertises itself as `Folkrace`.

## Phone control

With `FOLKRACE_ENABLE_BLE` set to `1`, connect using a BLE UART app such as
**nRF Connect** or **Serial Bluetooth Terminal**. The firmware advertises the
standard Nordic UART Service; write plain-text commands to its RX characteristic
and enable notifications on TX to receive responses. Each command may be sent
as one BLE write (a trailing newline is optional).

```text
HELP
READY
DRIVE 120 -30
START
STOP
```

`DRIVE <throttle> <steering>` uses values from `-255` to `255` and performs
arcade mixing. `TANK <left> <right>` controls each wheel directly. A lost
control command stops both motors after `FOLKRACE_COMMAND_TIMEOUT_MS`; this is
intentional safety behavior. `STATUS` reports the connection/state and `IMU`
prints the latest acceleration and gyro sample when the IMU is enabled.

For an MPU-6050, enable `FOLKRACE_ENABLE_IMU`, install **FastIMU**, and set the
I2C pins/address in `config.h`. Put custom wall-following, PID, and sensor code
in the hooks at the top of `Android.ino` (`onRobotRunning` and `onIMUUpdated`).
