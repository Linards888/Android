# Setup

See `docs/Necessities.md` first for exactly what hardware/software/libraries
you need, and `docs/Arhitecture.md` for how the code fits together.

## 1. Get the project

```powershell
git clone https://github.com/Linards888/Android
cd Android
```

The main sketch is `Android.ino`. It only contains the state machine and the
values you'll retune often — everything else lives in `src/`, and you
shouldn't need to open it. `config.h` picks which robot you're building for;
the actual pin/sensor/motor settings for each robot live under `configs/`.

## 2. Install ESP32 development tools

### Arduino IDE

1. Install [Arduino IDE](https://www.arduino.cc/en/software) (2.x recommended).
2. Open **File > Preferences** and add this URL to **Additional Boards Manager
   URLs**:

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Open **Tools > Board > Boards Manager**, search for **esp32**, and install
   **esp32 by Espressif Systems**.
4. Connect the ESP32 by USB. Install its USB-to-serial driver if no serial
   port appears in **Tools > Port**.

> [!CAUTION]
> Use an ESP32 board, not an Arduino Uno/Nano — this firmware uses ESP32-only
> features (Preferences, Bluetooth Low Energy). See `docs/Necessities.md`.

### Arduino CLI

`sketch.yaml` at the project root already sets a default board (`esp32:esp32:esp32c3`)
and port — edit it if yours differ. Then:

```powershell
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "VL53L0X" "FastIMU" "ESP32Servo"   # only the ones your config actually needs
arduino-cli compile
arduino-cli upload
```

### Arduino IDE for VS Code (Arduino Community Edition)

> [!NOTE]
> Coming soon!

## 3. Install libraries

In Arduino IDE, open **Tools > Manage Libraries** and install only the
libraries your active config's features actually need — see the table in
`docs/Necessities.md`. `Wire`, `Arduino`, `Preferences`, and the BLE headers
are all part of the `esp32` board package from step 2; nothing extra to
install for those.

## 4. Pick (or create) your robot's config

Open `config.h`. It should look like this:

```cpp
#define ACTIVE_ROBOT_CONFIG "configs/config_example_2motor.h"
```

- **Building for a robot that already has a config file** (check `configs/`)
  — just point `ACTIVE_ROBOT_CONFIG` at it.
- **Building for a new robot** — copy `configs/config_template.h` to
  `configs/config_<yourbot>.h`, fill it in (every option is commented right
  there: board pins, which sensors, drive layout, which optional features),
  and point `ACTIVE_ROBOT_CONFIG` at your new file.

`configs/config_example_2motor.h` and `configs/config_example_1motor_servo.h`
are two complete, different, working examples — useful as a second reference
alongside the template.

> [!WARNING]
> Enable only the sensors/features you've actually wired up. Turning on
> `SENSOR_TOF_ENABLED` without a VL53L0X connected just means `dist()` calls
> for that sensor return stale/invalid readings (and `tof_init()` logs an
> init failure) — it won't damage anything, but your logic will be reacting
> to nothing.

## 5. Board & upload settings (Arduino IDE)

1. **Tools > Board** — select your exact ESP32 variant.
2. **Tools > Port** — select the robot's serial port.
3. Recommended **Tools** settings:
   - USB CDC On Boot: `Enabled`
   - Erase All Flash Before Sketch Upload: `Disabled` (enabling this wipes
     any saved BLE-tuned parameters between uploads)
   - Upload Speed: as high as your board/cable reliably supports
4. Set the Serial Monitor baud rate to **115200** to match `Serial.begin()`
   in `Android.ino`.

## 6. Upload

1. Connect the board by USB and select its port.
2. Click **Upload**, wait for it to finish.
3. Open the Serial Monitor at 115200 baud — you should see sensor init
   messages, then `<ROBOT_NAME> ready. Try 'help' over BLE/Serial.`

If it doesn't build, check that every library your enabled features need
(step 3 / `docs/Necessities.md`) is actually installed, and that
`config.h`'s `ACTIVE_ROBOT_CONFIG` points at a file that exists.

## 7. First connection

1. If `FEATURE_BLUETOOTH` is on, the board advertises as whatever
   `ROBOT_NAME` your active config sets (`"Folkrace"` by default). Connect
   with any BLE terminal app (nRF Connect, LightBlue, ...).
2. Send `help` to see every command. Send `list` to see every tunable
   parameter and its current value.
3. Send `ready`, then `start` — the robot counts down (`start_delay`, in ms,
   4500 by default) and then runs.
4. Send `stop` any time to cut the motors and go back to idle.

See `docs/Calibration.md` for tuning sensor thresholds and steering weights,
and `docs/Arhitecture.md` for what every command/parameter actually does and
how to add your own.
