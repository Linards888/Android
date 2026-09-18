# Setup

Before you start, check [Necessities.md](Necessities.md) for what you need.

## Step 1 - Get the code

```powershell
git clone https://github.com/Linards888/Android
cd Android
```

Three things live in this folder:

- **`Android.ino`** - the main sketch. You rarely need to edit this.
- **`config.h`** - your robot's description (pins, sensors, motors,
  features, starting tuning values). You'll edit this every time.
- **`src/`** - drivers and plumbing. You shouldn't need to open this.

## Step 2 - Install the Arduino IDE and the ESP32 board package

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Open **File > Preferences**, and add this to **Additional Boards Manager
   URLs**:

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Open **Tools > Board > Boards Manager**, search for `esp32`, and install
   **esp32 by Espressif Systems**.
4. Plug in your ESP32 over USB. If no port shows up under **Tools > Port**,
   install its USB-to-serial driver (CP210x or CH340, depending on the board).

> [!CAUTION]
> Use an ESP32 board, not an Arduino Uno/Nano - see [Necessities.md](Necessities.md) for why.

## Step 3 - Install libraries

Open **Tools > Manage Libraries** and install only what the features you're
turning on need - see the table in [Necessities.md](Necessities.md). Don't
install libraries for features you're leaving off; they're not needed.

## Step 4 - Configure `config.h`

Open `config.h`. It's laid out in the order you should fill it in:

1. **Hardware** - set `Is_Esp32` to `1`, pick your distance sensor(s), pick
   exactly one drive layout, turn on whichever optional features you want.
2. **Wiring** - fill in the pin numbers, I2C addresses and mounting angles
   for whatever you turned on in step 1. Name sensors `front`, `left` and
   `right` where they apply - the driving algorithm looks them up by
   exactly these names (see [Algorithm.md](Algorithm.md)).
3. **Starting tuning values** - the defaults are a reasonable starting
   point. Leave them as-is for your first upload; you'll tune them live
   over BLE once the robot is on the bench (step 6).

Leaving everything at `0` (the state you clone the repo in) is intentional:
the build refuses to compile with a clear error message telling you exactly
what to set, instead of silently producing a robot that can't drive. If you
see a `#error` when compiling, that's `config.h` telling you what's missing
or conflicting - fix it there, not in the code.

## Step 5 - Board settings and upload

1. Select your board under **Tools > Board** and your port under **Tools > Port**.
2. Under **Tools**, set:
   - **USB CDC On Boot:** `Enabled`
   - **Erase All Flash Before Sketch Upload:** `Disabled` (enabling this
     would also wipe any saved BLE tuning values - see `Memory` in `config.h`)
   - **Upload Speed:** as high as your board supports (`115200` is a safe default)
3. Click **Upload**.

If it fails to compile, read the error message first - most come straight
from `config.h` (see step 4) rather than a bug in the firmware itself.

## Step 6 - First run, on the bench

1. **Wheels off the ground (or motors disconnected) for this first run.**
2. Open the Serial Monitor at **115200 baud**. `Folkrace ready.` means every
   enabled sensor initialized and `config.h` passed all its checks.
3. If `Is_blueTooth` is on, connect with any generic BLE serial/terminal app
   (the robot advertises as `BLE_DEVICE_NAME` from `config.h`, default
   `Folkrace`). Send `help` for the full command list.
4. Typical bring-up sequence over BLE:

   ```
   ready
   calibrate       (re-calibrates the IMU, if Is_IMU is on)
   state           (see current values)
   k p 2.0         (tune live, e.g. steering Kp)
   speed forward 120
   save            (write current values to flash - they survive a reboot)
   start           (counts down, then starts driving)
   ```

   See [BLE_Commands.md](BLE_Commands.md) for the complete list.
5. Only once you're happy with how it behaves on the bench, put the wheels
   down and try it on the actual track.

## Troubleshooting

| Symptom | Likely cause |
| --- | --- |
| Won't compile, `#error "No board selected!"` / `"No motor configuration selected!"` / similar | `config.h` step 1 isn't finished - see step 4 above. |
| Won't compile, some other library-related error | You enabled a feature in `config.h` without installing its library (step 3), or the library version is too old/new. |
| Compiles, but `TOF init failed: <name>` on Serial | Check that sensor's XSHUT pin and wiring. |
| Robot spins in place / drives backwards | Check `MOTOR_LIST` pin order in `config.h`, or toggle the BLE `reverse_drive` command. |
| Steering over/under-corrects, oscillates | Tune `k p`/`k i`/`k d` live over BLE - see [Algorithm.md](Algorithm.md) for what each does. |
| Sharp sensor readings look wrong/inverted | Needs its two-point calibration - see [Calibration.md](Calibration.md). |
| BLE console not responding | Make sure `Is_blueTooth` is `1` and you reconnected after the last upload (the device name/UUIDs don't change, but old connections can go stale). |

For anything else, check the exact compiler error or Serial output first -
almost every failure mode here traces back to one line in `config.h`.
