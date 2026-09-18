# Necessities

What you actually need — hardware, software, and libraries — before this
firmware will build and run. See `docs/Setup.md` for the step-by-step
install/upload process, and `docs/Arhitecture.md` for how the code is put
together.

## Hardware

- **An ESP32 board.** This firmware uses `Preferences` (flash storage) and
  Bluetooth Low Energy, both ESP32-only — an Arduino Uno/Nano/Mega will not
  work, full stop. Developed and tested against an ESP32-C3 ("mini" style
  board); any ESP32 variant with enough free GPIO for your sensors/motors
  should work, but pin numbers in your `configs/config_*.h` will differ.
- **Motors + driver, matching your `MOTOR_KIND`:**
  - `MOTOR_BRUSHED` — a DC gearmotor per `MOTOR_LIST` entry, through an
    H-bridge driver (e.g. TB6612, DRV8833, L298N). Each entry needs two
    PWM-capable GPIOs.
  - `MOTOR_BRUSHLESS` — a brushless motor + ESC per `MOTOR_LIST` entry, one
    PWM-capable GPIO each (ESC signal wire). **Most ESCs need to be armed**
    (a specific low-throttle signal on power-up) before they'll spin —
    check your ESC's manual; `drive_init()` currently just attaches the
    signal pin and doesn't run an arming sequence, since these vary a lot
    between ESCs.
- **A steering servo**, if `STEERING_MODE` is `STEERING_SERVO` — one
  PWM-capable GPIO.
- **Distance sensors**, matching whatever you enable:
  - Sharp/analog IR — one ADC-capable GPIO per sensor.
  - VL53L0X (ToF, I2C) — shares `PIN_SDA`/`PIN_SCL`. Running more than one
    on the bus needs a real `xshutPin` per sensor (a free GPIO each) so
    they can be woken one at a time and given unique addresses; a single
    sensor doesn't need one (`PIN_NONE`).
  - Ultrasonic (HC-SR04 style) — one GPIO for `trig`, one for `echo`, per
    sensor. Note these are usually 5V-tolerant on the echo pin — check your
    specific module against your ESP32's GPIO voltage tolerance (3.3V) and
    level-shift if needed.
- **IMU**, if `FEATURE_IMU` is on — a BMI160 (or another FastIMU-supported
  chip; you'll need to change the type in `src/IMU/IMU_logic.cpp` if it's
  not a BMI160) on the same I2C bus as any ToF sensors.
- **Enough power for your motors separate from the ESP32's logic supply**
  where your driver/ESC needs it — brushed gearmotors and brushless ESCs
  under load can brown out a shared 5V/3.3V rail and reset the ESP32 mid-run.

## Software

- **Arduino IDE** (2.x recommended) or **arduino-cli**. `sketch.yaml` at the
  project root already points arduino-cli at `esp32:esp32:esp32c3` — change
  `default_fqbn` there if you're on a different ESP32 variant.
- **The `esp32` board package** (by Espressif Systems), added via the
  Boards Manager URL in `docs/Setup.md`. This provides the ESP32 core,
  `BLEDevice`/`BLEServer`/`BLEUtils` (BLE), and `Preferences` — you do not
  install those separately.

## Libraries, by feature

Only install what your `configs/config_*.h` actually turns on — nothing
else needs to compile.

| You enabled... | Install this library |
| --- | --- |
| `SENSOR_TOF_ENABLED` | **VL53L0X** by Pololu |
| `FEATURE_IMU` | **FastIMU** |
| `STEERING_MODE == STEERING_SERVO`, or `MOTOR_KIND == MOTOR_BRUSHLESS` | **ESP32Servo** |
| `FEATURE_BLUETOOTH` | nothing extra — part of the `esp32` board package |
| `SENSOR_SHARP_ENABLED`, `SENSOR_ULTRASONIC_ENABLED`, `FEATURE_MEMORY` | nothing extra — plain `analogRead`/`pulseIn`/`Preferences` |

`Wire` and `Arduino` core APIs are always available from the board package.

## A PC-side BLE client

You'll want something that can connect to a BLE peripheral and send/receive
text over a single read/write/notify characteristic to actually use the
`get`/`set`/`list`/... commands — a generic BLE terminal app (e.g. nRF
Connect, LightBlue) on your phone works well for this and needs no extra
setup. The characteristic UUID is in `src/BLE/RobotBLE.cpp` if a client asks
for it explicitly.
