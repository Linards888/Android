# Configuration

This is the one file to read to set up (or add) a robot. No architecture,
no "why" — just what to do and what every setting means. (If you want the
"why"/how-it-all-fits-together explanation, that's `docs/Arhitecture.md`.)

## The two files involved

- **`config.h`** (project root) — picks which robot is active. You will
  normally only ever touch its one `ACTIVE_ROBOT_CONFIG` line.
- **`configs/config_<yourbot>.h`** — one file per physical robot: pins,
  sensors, motors, features. This is where you'll spend your time.

You never edit `Android.ino` or anything in `src/` to configure a robot.

## Step-by-step: setting up a new robot

1. **Copy the template.**
   Duplicate `configs/config_template.h` as `configs/config_<yourbot>.h`
   (e.g. `configs/config_my2wd.h`). Give it a name that identifies the
   physical robot, not the track/event.

2. **Fill in section 1 (identity).**
   ```cpp
   #define ROBOT_NAME    "MyRobot"   // shows up when you scan for it over BLE
   #define ROBOT_STORAGE "myrobot"   // must be unique per robot — see warning below
   ```

3. **Turn on the features you actually have** (section 2). Everything
   defaults to a safe "off" in the template — flip each one to `1` only if
   the hardware/behavior is actually present. Leaving a feature off means
   its code isn't even compiled in, so leftover `0`s cost nothing.

4. **Enable and list your distance sensors** (section 3). Set the relevant
   `SENSOR_*_ENABLED` to `1`, set `PIN_SDA`/`PIN_SCL` if you're using I2C
   sensors (ToF) or the IMU, then fill in that sensor type's `_LIST` macro —
   one line per physical sensor. Set `PRIMARY_FRONT_SENSOR` to the name of
   whichever sensor has `ROLE_FRONT`.

5. **Fill in IMU settings** (section 4) — only if `FEATURE_IMU` is `1`.

6. **Pick your chassis layout** (section 5): `DRIVE_TOPOLOGY`,
   `STEERING_MODE`, `MOTOR_KIND`, then list your motors in `MOTOR_LIST`. If
   `STEERING_MODE` is `STEERING_SERVO`, also set `STEERING_SERVO_PIN`.

7. **Point `config.h` at your new file:**
   ```cpp
   #define ACTIVE_ROBOT_CONFIG "configs/config_myrobot.h"
   ```

8. **Upload.** If you got a required setting wrong (missing pin, invalid
   combination), the build will fail with a clear `#error` message telling
   you exactly what to fix — see [Troubleshooting](#troubleshooting-build-errors)
   below.

That's the whole process. Nothing outside your new config file and that one
line in `config.h` ever needs to change to add a robot.

**Switching between robots you already have configs for** is just step 7 +
8 — change the one line, re-upload.

---

## Full parameter reference

### 1) Identity

| Parameter | Type | Example | Meaning |
| --- | --- | --- | --- |
| `ROBOT_NAME` | string | `"Folkrace"` | The BLE advertised name — what you'll see when scanning for the robot from a phone/PC. |
| `ROBOT_STORAGE` | string | `"folkrace"` | The flash (NVS) namespace saved settings live under. **Must be unique per robot** if you ever flash more than one robot's config to boards that might share... actually it just needs to not collide on a given board; give every robot its own value as a habit so `save`/`defaults` on one never bleeds into another. |

### 2) Optional features

All are `0` (off, compiled out) or `1` (on). Turning a feature off removes
its code from the build entirely — there's no runtime cost to leaving
something off, so only turn on what you actually have wired up.

| Parameter | Turns on | Needs |
| --- | --- | --- |
| `FEATURE_BLUETOOTH` | BLE tuning/commands (`src/BLE`) | nothing extra |
| `FEATURE_IMU` | gyro/accel readings (`src/IMU`) | section 4 filled in, `PIN_SDA`/`PIN_SCL` set |
| `FEATURE_MEMORY` | remembers tuned parameters across power cycles (`src/Memory`) | nothing extra |
| `FEATURE_TELEMETRY` | CSV stream to a PC over Serial (`src/Telemetry`) | nothing extra |
| `FEATURE_SPACE_AWARENESS` | **not implemented** — placeholder only | leave `0` |

### 3) Distance sensors

First, enable whichever physical sensor types you have:

| Parameter | Turns on |
| --- | --- |
| `SENSOR_SHARP_ENABLED` | analog IR sensors (e.g. Sharp GP2Y0A21) |
| `SENSOR_TOF_ENABLED` | VL53L0X time-of-flight sensors (I2C) |
| `SENSOR_ULTRASONIC_ENABLED` | HC-SR04-style ultrasonic sensors |

If `SENSOR_TOF_ENABLED` is `1` (or `FEATURE_IMU` is `1`, since the IMU is
also I2C), set the I2C bus pins:

```cpp
#define PIN_SDA <pin>
#define PIN_SCL <pin>
```

Then, for each enabled type, define its `_LIST` macro — one `X(...)` entry
per physical sensor. **Sensor names must be unique across all three lists
combined.**

| List macro | Format | Column meaning |
| --- | --- | --- |
| `SHARP_SENSOR_LIST` | `X(name, pin, angleDeg, role, weight)` | `pin`: the analog pin it's wired to |
| `TOF_SENSOR_LIST` | `X(name, xshutPin, i2cAddress, angleDeg, role, weight)` | `xshutPin`: `PIN_NONE` if it's the only ToF sensor on the bus; a real pin + unique `i2cAddress` per sensor if you have more than one |
| `USONIC_SENSOR_LIST` | `X(name, echoPin, trigPin, angleDeg, role, weight)` | `echoPin`/`trigPin`: the two pins HC-SR04-style sensors need |

Shared columns, same meaning in every list:

| Column | Meaning |
| --- | --- |
| `name` | how you'll refer to this sensor in code (`dist("name")`) and over BLE (`w_name`). Must be unique across all sensors. |
| `angleDeg` | where the sensor physically points, in degrees (0 = straight ahead, negative = left, positive = right). Informational/for your own use — not consumed by the built-in logic yet. |
| `role` | `ROLE_STEER` (feeds the weighted steering-error sum), `ROLE_FRONT` (drives speed-scaling + the escape maneuver — give this to exactly one sensor), or `ROLE_AUX` (available via `dist(name)`, otherwise unused) |
| `weight` | this sensor's multiplier in the steering-error sum. Irrelevant (but still required as a column) for `ROLE_FRONT`/`ROLE_AUX` sensors — `0.0f` is conventional there. Individually BLE-tunable later as `w_<name>`. |

Finally:

```cpp
#define PRIMARY_FRONT_SENSOR "front"   // the name of your ROLE_FRONT sensor
```

### 4) IMU

Only needed if `FEATURE_IMU` is `1`:

```cpp
#define IMU_I2C_ADDRESS 0x68   // BMI160's I2C address — 0x68 or 0x69 depending on wiring
```

`PIN_SDA`/`PIN_SCL` (set in section 3) are what the IMU actually uses to
talk to the board — it shares the bus with any ToF sensors.

### 5) Drive

Three choices that together describe your chassis, plus the motor list:

| Parameter | Options | Meaning |
| --- | --- | --- |
| `DRIVE_TOPOLOGY` | `DRIVE_1_MOTOR`, `DRIVE_2_MOTOR`, `DRIVE_4_MOTOR` | how many independently-driven propulsion motors you have |
| `STEERING_MODE` | `STEERING_NONE`, `STEERING_DIFFERENTIAL`, `STEERING_SERVO` | how the robot turns — see table below |
| `MOTOR_KIND` | `MOTOR_BRUSHED`, `MOTOR_BRUSHLESS` | brushed DC motor through an H-bridge, or brushless through an ESC |

**`STEERING_MODE` in plain terms:**

| Value | What it means | Requires |
| --- | --- | --- |
| `STEERING_NONE` | robot can't turn — every motor always gets the same speed, `steer` is ignored | — |
| `STEERING_DIFFERENTIAL` | "tank steering" — no separate steering part; left motors and right motors run at different speeds to turn | ≥2 motors, each tagged `SIDE_LEFT`/`SIDE_RIGHT` in `MOTOR_LIST` |
| `STEERING_SERVO` | normal RC-car layout — all drive motors run the same speed, a separate servo steers | `STEERING_SERVO_PIN` defined |

Then list every motor:

```cpp
#define MOTOR_LIST \
    X(name, side, pinA, pinB) \
    ...
```

| Column | Meaning |
| --- | --- |
| `name` | just a label, used internally (`motor_<name>`) |
| `side` | `SIDE_LEFT` / `SIDE_RIGHT` — only meaningful (and required) for `STEERING_DIFFERENTIAL`; use `SIDE_NONE` otherwise |
| `pinA`, `pinB` | **brushed**: the two H-bridge input pins. **brushless**: `pinA` = ESC signal pin, `pinB` = `PIN_NONE` (unused) |

If `STEERING_MODE` is `STEERING_SERVO`, also set:

```cpp
#define STEERING_SERVO_PIN <pin>          // required
#define STEERING_SERVO_CENTER_US 1500     // optional — default shown, rarely needs changing
#define STEERING_SERVO_RANGE_US  500      // optional — +/- microseconds from center at full steering lock
```

---

## Symbolic constants cheat sheet

These are all plain `#define`s from `src/Utils/HardwareEnums.h`, already
available in every config file — you never need to define these yourself,
just use them.

| Constant | Value | Used for |
| --- | --- | --- |
| `DRIVE_1_MOTOR` / `DRIVE_2_MOTOR` / `DRIVE_4_MOTOR` | 1 / 2 / 4 | `DRIVE_TOPOLOGY` |
| `STEERING_NONE` / `STEERING_DIFFERENTIAL` / `STEERING_SERVO` | 0 / 1 / 2 | `STEERING_MODE` |
| `MOTOR_BRUSHED` / `MOTOR_BRUSHLESS` | 0 / 1 | `MOTOR_KIND` |
| `SIDE_NONE` / `SIDE_LEFT` / `SIDE_RIGHT` | 0 / 1 / 2 | `MOTOR_LIST`'s `side` column |
| `ROLE_STEER` / `ROLE_FRONT` / `ROLE_AUX` | 0 / 1 / 2 | every sensor list's `role` column |
| `PIN_NONE` | 255 | "this pin isn't used" — the unused `xshutPin`, brushless `pinB`, or any pin you haven't wired yet |

---

## Troubleshooting build errors

The config system checks itself at compile time — these are the specific
`#error`s you might hit and what they mean:

| Error | Cause | Fix |
| --- | --- | --- |
| `This firmware targets ESP32 boards only...` | Wrong board selected in the IDE | Tools → Board → pick an ESP32 board |
| `Your config is missing DRIVE_TOPOLOGY / STEERING_MODE / MOTOR_KIND / MOTOR_LIST` | One of those four wasn't defined | Fill in all of section 5 |
| `A single motor can't do differential steering...` | `DRIVE_TOPOLOGY` is `DRIVE_1_MOTOR` with `STEERING_MODE` set to `STEERING_DIFFERENTIAL` | Use `STEERING_SERVO` or `STEERING_NONE` for a 1-motor chassis, or add a second motor |
| `STEERING_MODE is STEERING_SERVO but STEERING_SERVO_PIN isn't defined` | Forgot the servo pin | Add `#define STEERING_SERVO_PIN <pin>` |
| `FEATURE_IMU is on but IMU_I2C_ADDRESS isn't defined` | Section 4 incomplete | Add `#define IMU_I2C_ADDRESS 0x68` (or your address) |

Beyond compile-time errors, two easy-to-miss mistakes that build fine but
misbehave on the bench:

- **Duplicate sensor names** across `SHARP_SENSOR_LIST`/`TOF_SENSOR_LIST`/
  `USONIC_SENSOR_LIST` — `dist("name")` will just find whichever one comes
  first, silently.
- **No sensor with `ROLE_FRONT`**, or `PRIMARY_FRONT_SENSOR` misspelled
  relative to the actual sensor `name` — `dist_front()` will return `-1`
  forever, and speed-scaling/the escape maneuver simply never trigger.

---

## Worked examples

Two complete, different, working configs are in `configs/`:

- **`config_example_2motor.h`** — 2 brushed motors, tank (differential)
  steering, 2x Sharp IR for wall-following + 1x VL53L0X up front.
- **`config_example_1motor_servo.h`** — classic RC-car layout: 1 drive
  motor + steering servo, single front VL53L0X, no side sensors.

Both produce a fully working robot from the exact same `Android.ino` and
`src/` — only the config file (and `config.h`'s one line) differ. Use
whichever is closer to your chassis as a starting point instead of the bare
template.
