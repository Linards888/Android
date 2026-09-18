# Necessities

Everything below is organized by feature - only get what the features you
actually plan to enable in `config.h` need.

## Board (required)

- An **ESP32** development board. Any variant works (the plain "ESP32
  DevKit" style boards are the cheapest and easiest to find) - this
  firmware needs ESP32 features (Bluetooth Low Energy, the `Preferences`
  flash storage API, and hardware PWM) and will not run on an Arduino
  Uno/Nano/Mega.
- A USB cable that actually carries data (not just power) to program it.

## Drive (required - pick one)

| Layout | Motors | Notes |
| --- | --- | --- |
| `OneMotor` | 1 | Needs `Is_servo` too, or it can't steer at all |
| `TwoMotors` | 2 | Steers by spinning the two sides at different speeds |
| `tank` | 4 | Skid-steer, same idea as `TwoMotors` but with 4 motors |

For any of the above you also need:

- A **motor driver** with two PWM-capable input pins per motor (e.g.
  **TB6612FNG**, **DRV8833**) - this firmware drives motors "IN/IN" style
  (PWM on one pin for forward, PWM on the other for reverse), not the
  separate PWM+direction-pin style some other drivers use.
- A battery appropriate for your motors and driver, plus a way to power the
  ESP32 (either the same battery through a regulator, or a separate supply).
- If `Is_servo` is on: a hobby servo, and the **ESP32Servo** library (see
  below).

## Distance sensors (at least one strongly recommended)

Pick any combination:

| `config.h` flag | Sensor | Library needed |
| --- | --- | --- |
| `Is_TOF` | VL53L0X time-of-flight | **VL53L0X by Pololu** |
| `Is_Sharp` | Analog Sharp IR (e.g. GP2Y0A21) | none - plain `analogRead` |
| `Is_Ultrasonic` | HC-SR04 style trigger/echo | none - plain `pulseIn` |

You can mix types freely (e.g. a TOF sensor facing front, two Sharp sensors
angled left/right).

## Optional features

| `config.h` flag | What it needs |
| --- | --- |
| `Is_blueTooth` | Nothing extra - BLE is built into the ESP32 board package. You'll also want a BLE serial/terminal app on your phone or PC to talk to it (any generic "BLE UART" app works). |
| `Is_IMU` | A gyro/accelerometer breakout your `IMU_MODEL` chip choice matches (see `config.h`), and the **FastIMU** library. |
| `Memory` | Nothing extra - uses the ESP32's built-in flash (`Preferences`), on by default. |

## Software

- **Arduino IDE** (2.x recommended) or Arduino CLI.
- The **esp32** board package (by Espressif Systems), installed via the
  Boards Manager - see [Setup.md](Setup.md).
- Only the libraries your enabled features actually need, from the tables
  above - installed via the Library Manager.

## Tools

- A small screwdriver set for mounting sensors/motors.
- A multimeter is handy but not required.
- For Sharp IR sensors specifically: a tape measure or ruler, to calibrate
  distance readings (see [Calibration.md](Calibration.md)).
