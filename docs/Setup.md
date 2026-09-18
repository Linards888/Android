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
| Servo (`Is_servo`) | ESP32Servo |
| BLE (`Is_blueTooth`) | Provided by the installed ESP32 board package |

`Wire`, `Arduino`, and `Preferences` are supplied by the Arduino/ESP32
framework and do not need separate installation. Sharp IR and ultrasonic
(HC-SR04 style) sensors are driven directly with `analogRead`/`pulseIn` and
don't need a library either.

## 4. Configure your robot.

> [!NOTE]
> 1. Go to [`config.h`](../config.h) before building. Change the `0` and `1` values to describe the robot you actually built.
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
> 4. Enable optional features such as `Is_blueTooth`, `Is_IMU`.
> 5. Update the sensor lists, motor pins. Name your sensors `front`, `left`,
>    and/or `right` - the built-in wall-following logic looks them up by
>    exactly those names, whatever sensor technology provides them.
> 6. If `Is_IMU` is on, set `IMU_MODEL` in `config.h` to the exact chip you
>    have wired up (e.g. `MPU6500`) - see the FastIMU library README for the
>    full list of supported chips.

Leaving `config.h` at its default (everything `0`) is intentional: the build
will fail with a clear `#error` telling you what to set, rather than silently
compiling something that can't drive.

## 5. Bring it up on the bench before you race it

1. Upload with no motors connected (or wheels off the ground) the first time.
2. Open the Serial Monitor at **115200 baud** - `Folkrace ready.` on boot
   means config.h passed all its checks and every enabled sensor initialized.
3. If `Is_blueTooth` is on, connect with a BLE serial app (advertised name:
   whatever you set `BLE_DEVICE_NAME` to, default `Folkrace`) and send `help`
   to see the full command list, `state` to see current parameter values.
4. Typical bring-up: `ready`, then `calibrate` (recalibrates the IMU if one
   is enabled), then tune `k`/`speed`/`dist` values live and `save` once
   you're happy - saved values survive a reboot automatically.

## 6. Upload the sketch

1. Connect Microcontroller to PC and select **Serial Port**.
2. CLick **Upload**, and wait for the code to upload.
3. If there are errors, check the exact `#error`/`#warning` text from the
   compiler first - most come directly from an inconsistent `config.h`
   selection (e.g. two motor types selected, or a feature that needs a
   library you haven't installed).

> [!NOTE]
> When Bluetooth is enabled, the firmware advertises itself under whatever
> `BLE_DEVICE_NAME` is set to in `config.h` (default `Folkrace`).
