# Architecture

This document explains how the codebase fits together: what lives where, why
it's split the way it is, and — most importantly — exactly what to touch for
each kind of change you'll want to make. If you only read one section, read
["How do I…"](#how-do-i) below.

## The three layers

```
configs/config_<robot>.h   <- WHAT you have: pins, sensors, motors, features
config.h                   <- WHICH config is active right now (one line)
Android.ino                <- setup() + the RunningState switch, nothing else
src/*                      <- HOW everything works, so the two files above
                               don't have to
```

You will spend almost all of your time in the top two layers, plus the
"QUICK TUNABLES" block near the top of `src/Core/RobotLogic.h`. `src/` exists
so you don't have to look at it.

## Why a config *selector* instead of one config file

Most Folkrace codebases end up with either one config file per robot copied
into a whole separate project (painful to keep in sync when you fix a bug),
or one giant config file full of `#ifdef ROBOT_A ... #elif ROBOT_B` blocks
(painful to read). This project does neither: `config.h` is three lines that
just pick which file to include:

```cpp
#define ACTIVE_ROBOT_CONFIG "configs/config_example_2motor.h"
#include ACTIVE_ROBOT_CONFIG
```

Every physical robot you own gets its own small file under `configs/`. You
never copy `Android.ino` or `src/` — there is exactly one copy of the actual
logic, shared by every robot. Switching robots (or building the same sketch
for two robots back to back) is changing one line and re-uploading.

`configs/config_template.h` documents every option. `configs/config_example_*.h`
are two real, different, working robots (2-motor tank drive, and a 1-motor
RC-car layout) so you can see the pattern rather than just read about it.

## Module map

| Module | Files | Responsibility |
| --- | --- | --- |
| Sketch glue | `Android.ino` | `setup()` call order + the `RunningState` switch in `loop()`. Nothing else. |
| Robot logic | `src/Core/RobotLogic.h` | Quick-tunable defaults, `registerParams()`, `run_line_following()`, `run_debug_logging()`, `on_state_enter()`, `service_tick()`. |
| State | `src/Utils/RobotState.h/.cpp` | The single `RobotState state` struct everything reads/writes. No logic. |
| Params | `src/Utils/ParamRegistry.h/.cpp` | Name → pointer registry. Makes a value BLE-tunable + saveable. |
| PID | `src/Utils/PIDAlgorithm.h` | Generic PID loop, reads gains live from `state.pid`. |
| Sensors | `src/Sensors/*` | `dist(name)`, `dist_front()`, `sensors_steer_error()`. Hides Sharp/ToF/ultrasonic differences. |
| IMU | `src/IMU/IMU_logic.h/.cpp` | `imu_pitch()`, `imu_yaw()`, accel/gyro getters. |
| Drive | `src/drive/Drive.h/.cpp` | `drive_set(throttle, steer)`. Hides 1/2/4-motor, brushed/brushless, servo/differential steering. |
| Calibration | `src/Calibrations/Calibration.h/.cpp` | Live sensor dump while `RunningState::CALIBRATING`. |
| Memory | `src/Memory/StateStore.h/.cpp` | Saves/restores every `ParamRegistry` entry to flash (`Preferences`). |
| Telemetry | `src/Telemetry/Telemetry.h/.cpp` | CSV stream to a PC over Serial. |
| Space awareness | `src/spaceAverenes/space.h/.cpp` | **Not implemented.** Empty on purpose — scaffolding for a future track-mapping feature. |
| BLE | `src/BLE/RobotBLE.h/.cpp`, `src/BLE/commands.h/.cpp` | Everything Bluetooth. The only module that knows BLE exists. |

Every module above is included from `src/Core/RobotLogic.h` behind its
feature's `#if FEATURE_*` (or `#if SENSOR_*_ENABLED`), so code for a feature
you don't use doesn't even get compiled in.

## The state machine

`RobotState::running_state` (`src/Utils/RobotState.h`) is one of:

```
IDLE -> READY -> COUNTDOWN -> RUNNING
  ^        |
  +--- CALIBRATING
MANUAL_FORWARD / MANUAL_BACKWARD  (debug driving, entered from anywhere via BLE)
```

`Android.ino`'s `loop()` does two things every tick, in order:

```cpp
void loop() {
    dt.update();
    float delta = dt.get();

    service_tick(delta);       // see src/Core/RobotLogic.h

    switch (state.running_state) {
        ...
    }
}
```

1. `service_tick(delta)` (in `src/Core/RobotLogic.h`) does everything that
   needs to happen every tick *regardless* of state: servicing sensors/IMU/
   telemetry/space-awareness, saving to flash when `state.save_state` is set,
   applying the compiled-in defaults when `state.reset_defaults` is set,
   running the manual-180 command, printing debug logs, and — if
   `running_state` just changed since last tick — calling `on_state_enter()`
   exactly once.
2. The `switch` in `Android.ino` runs the *current* state's per-tick
   behaviour (`drive_stop()` in IDLE/READY, `calibration_run()` in
   CALIBRATING, the countdown timer, `run_line_following()` in RUNNING, fixed
   manual speeds in MANUAL_FORWARD/BACKWARD).

Commands (`src/BLE/commands.cpp`) only ever *set* `state.running_state` (or a
flag like `save_state`/`reset_defaults`/`do_manual_180`). They never contain
driving logic themselves — that keeps "what can happen" (`service_tick()` +
the switch in `loop()`) and "what triggers it" (BLE commands) cleanly
separated.

`on_state_enter()` is where you put "do this once, the moment we transition
into state X" logic — right now that's kicking off calibration
(`calibration_start()`) and resetting the PID's integral term when entering
RUNNING. Add a `case` there for any new one-shot-on-entry behaviour.

## ParamRegistry — the parameter registry

**What it is.** A tiny global table (`src/Utils/ParamRegistry.h/.cpp`)
mapping a short string name to a pointer to a real variable somewhere in
memory (almost always a field of `state`), plus that variable's type. It's
the single mechanism that makes "make this number tunable" a one-line change
instead of a four-place change.

**The problem it solves.** Without it, adding one tunable number means: add
a field to the state struct, add a case to a BLE "get" switch, add a case to
a BLE "set" switch, add lines to save it to flash, add lines to load it back.
Five places, every time, for one idea. `ParamRegistry` collapses all five
into:

```cpp
REGISTER_PARAM(state.pid.kp);
```

called once, from `registerParams()` in `src/Core/RobotLogic.h`. That single
line makes `kp`:

- **readable** over BLE/Serial: `get kp`
- **settable**: `set kp 1.4`
- **toggleable**, if it's a `bool`: `toggle scaled_speed`
- **listed**, with every other registered param and its current value: `list`
- **saved to flash and restored on boot automatically**, via
  `src/Memory/StateStore.cpp`, with zero changes to that file — it just
  iterates the registry and doesn't know `kp` exists any more than
  `commands.cpp` does.

**How it works internally.** `ParamRegistry` is a fixed-size array of
`ParamEntry { name, type, ptr }` (`MAX_PARAMS = 64`; raise it if you ever hit
the limit — `reg()` silently drops entries past it, so watch the boot log's
"loaded N saved parameter(s)" line if numbers look low). `ParamType` is a
small enum (`FLOAT, U8, U16, I16, U32, BOOL`) — every supported field type
has its own `ParamRegistry::reg(name, T&)` overload, so `REGISTER_PARAM` just
needs to pick the right one at compile time via C++ overload resolution; you
never specify the type yourself.

`REGISTER_PARAM(var)` expands to `ParamRegistry::reg(param_short_name(#var), var)`.
`#var` stringifies the expression (e.g. `"state.pid.kp"`); `param_short_name()`
is a tiny runtime helper that returns everything after the last `.`
(`"kp"`). That's why the BLE name is always just the field's own name — you
don't type it twice. `REGISTER_PARAM_NAMED("short_name", var)` skips that and
uses the literal string you give it — use it when the real name is longer
than 15 characters (the ESP32 NVS/flash key length limit that `Preferences`
enforces) or you just want something shorter on the wire, e.g.
`REGISTER_PARAM_NAMED("esc_trig_mm", state.escape_trigger_mm)`.

`get_as_string()` / `set_from_string()` do the type-generic read/write that
`get`/`set`/`StateStore` all build on — they `switch` on `e->type` and do the
right `snprintf`/`atof`/`atol` cast under the hood, so the calling code never
needs a type-specific branch.

**Its "commands"** are the generic BLE/Serial commands in
`src/BLE/commands.cpp` that operate on *any* registered param by name, not
just one: `get <name>`, `set <name> <value>`, `toggle <name>` (bools only),
`list` (dumps every registered name + current value). These four commands
never need to change when you add a new tunable — that's the entire point.

**How to use it.** Two situations:
- *You added a field to `RobotState.h` and want it BLE-tunable*: add one
  `REGISTER_PARAM(state.your_field);` line inside `registerParams()` in
  `src/Core/RobotLogic.h`. Done.
- *You're registering something generated from config* (like per-sensor
  weights, which don't exist as named fields until your config's sensor list
  expands them): register it from the owning module's own `_register_params()`
  function instead (see `sensors_register_params()` in the Sensors section
  below) and call that function once from `registerParams()`. This keeps
  "what sensors exist" entirely inside the Sensors module.

## BLE — architecture and how to send commands

**Goal of the design:** exactly one file (`src/BLE/RobotBLE.cpp`) knows BLE
exists. Every other module — commands, sensors, drive, telemetry — talks
through two narrow, transport-agnostic seams, so BLE could be swapped for
WiFi/ESP-NOW/USB-only later by touching only `RobotBLE.cpp`.

**The two seams:**

1. **Incoming (BLE → command):** A phone/PC BLE client writes a text string
   to a single GATT characteristic. `RobotBLEWriteCallbacks::onWrite()`
   copies the incoming bytes into a small local buffer, NUL-terminates it,
   splits it on the first space into `command` (e.g. `"set"`) and `args`
   (e.g. `"kp 1.4"`) using `strtok_r`, and calls
   `handle_command(command, args)` — the exact same function a Serial
   console (or any future transport) would call. BLE itself never contains
   any command logic.
2. **Outgoing (module → BLE):** Every module — `commands.cpp`, `Calibration`,
   `Drive`, `StateStore`, etc. — calls `notify(fmt, ...)`
   (`src/Utils/Notify.h`) instead of printing directly. `notify()` always
   echoes to `Serial`. At boot, `ble_init()` additionally calls
   `notify_set_sink(ble_notify_sink)`, registering itself as an *extra*
   output sink. From then on every `notify()` call also pushes the same text
   out over the BLE characteristic's `notify()` (GATT notification), with no
   `#if FEATURE_BLUETOOTH` guard needed anywhere except right at BLE's own
   init call.

**The GATT layout:**

- One `BLEService`, UUID `deadf33f-a6a6-5155-c0de-a6a6f33f0001`
- One `BLECharacteristic` inside it, UUID `dec887a8-991b-4501-9409-5b83528be174`,
  with `READ | WRITE | NOTIFY` properties — the *same* characteristic is used
  both to send commands (write) and receive replies/logs (notify/read).
- Advertised device name is your config's `ROBOT_NAME`.

**How to send a command, practically.** Connect any BLE terminal app (nRF
Connect, LightBlue, a custom phone app, a Python `bleak` script — anything
that speaks BLE GATT) to the robot by its advertised `ROBOT_NAME`, open the
one service, then on the one characteristic:

- **Write** a plain ASCII string, no line ending needed, e.g. `set kp 1.4`,
  `start`, `get pid_limit`, `list`. Max ~63 bytes (the internal buffer is 64
  bytes including the NUL terminator) — long enough for any real command.
- **Subscribe to notifications** (enable the CCCD) on the same characteristic
  to receive replies and any log lines (`notify()` output) as they happen.
- Reading the characteristic's current value gets you whatever text was
  *last* notified — subscribing is the normal way to actually watch a stream
  of output rather than polling reads.

The exact same commands also work unchanged over plain USB-Serial (115200
baud) — type a command, press enter, `Serial` is `notify()`'s always-on sink.
That's the easiest way to test commands on the bench without any BLE client
at all.

**Full command list** — see `cmd_help` / the `help` command for the live
list, or `src/BLE/commands.cpp`'s `command_list[]`. Action commands: `ready`,
`start`, `stop`, `calibrate`, `fwd`, `bwd`, `180`, `save`, `defaults`,
`state`. Generic parameter commands: `get`, `set`, `toggle`, `list` (see
ParamRegistry above). `help` lists every command name.

**Adding a new command that isn't just a number:** in `src/BLE/commands.cpp`,
write `static void cmd_whatever(char* args) { ... }`, add
`{"whatever", cmd_whatever},` to `command_list[]`. `args` is whatever text
followed the command word, already isolated for you — `nullptr` if none was
given. That's the entire process; nothing in `RobotBLE.cpp` changes.

## Calibration — architecture and usage

**What it is.** A diagnostic tool, not automatic tuning. While
`running_state == RunningState::CALIBRATING`, the robot repeatedly prints
every enabled sensor's live reading so you can watch the numbers and pick
good threshold values by hand — it does not compute or set anything for you.

**How you use it, end to end:**

1. Send `calibrate` over BLE/Serial. `state.running_state` becomes
   `CALIBRATING`. `service_tick()` notices the state changed and fires
   `on_state_enter(CALIBRATING)`, which calls `calibration_start()`.
2. `calibration_start()` records the start time and, if `FEATURE_IMU` is on
   *and* `state.imu_enabled` is true, also calls `imu_recalibrate()` — hold
   the robot still and level right at this point, since that's a real
   gyro/accel recalibration, not just a print loop.
3. Every tick while in `CALIBRATING`, `loop()`'s `switch` calls
   `calibration_run()`, which: calls `sensors_update()` so readings are
   fresh; every ~200ms, prints every enabled sensor's current value (`Sharp`
   sensors print their smoothed raw value, `ToF`/ultrasonic print real
   millimeters); after 3000ms total, prints `calibration done` and returns
   `true`.
4. When `calibration_run()` returns `true`, `loop()` moves
   `running_state` to `READY`.

**What to actually do during those 3 seconds:** move the robot (or hold a
wall/object) at the distances that matter to your track — your intended
"too close, escape now" distance, your intended "far enough, go full speed"
distance, typical left/right wall distances — and read off the sensor values
printed. Then, *after* calibration ends (you're free to do this any time,
not just right after), use `set <name> <value>` to actually apply what you
learned: `set esc_trig_mm 40`, `set dist_near 60`, `set dist_far 300`, `set
w_left -1.2`, and so on — every one of those is a normal `ParamRegistry`
entry (see the BLE/ParamRegistry sections). `save` afterwards if you want the
new numbers to survive a reboot.

**Architecture note:** `Calibration.cpp` only talks to the Sensors module
(`sensors_update()`, the per-type `*_all[]` arrays) and, optionally, the IMU
module. It has no knowledge of BLE or Drive — it's a pure "read sensors,
print them" loop, callable from any transport exactly like every other
command.

## Drive — commands and architecture

**The abstraction.** `src/drive/Drive.h` is the *entire* interface
`RobotLogic.h` (and, if you extend the algorithm, your own code) is allowed
to use:

```cpp
drive_init();                 // once, in setup()
drive_set(throttle, steer);   // throttle: -255(back)..255(fwd), steer: -255(left)..255(right)
drive_stop();                 // equivalent to drive_set(0, 0)
drive_do_180();                // debug: spin/turn around, blocking ~400ms
```

Nothing outside `Drive.cpp` ever touches a pin, an `analogWrite`, or a
`Servo` object directly. What `drive_set()` *does* with those two numbers
depends entirely on three config choices and one list, all from your
`configs/config_*.h`:

- **`DRIVE_TOPOLOGY`** — how many independently-driven motor outputs you
  have: `DRIVE_1_MOTOR`, `DRIVE_2_MOTOR`, `DRIVE_4_MOTOR`.
- **`MOTOR_KIND`** — `MOTOR_BRUSHED` (H-bridge, two digital pins per motor,
  driven with `analogWrite` PWM) or `MOTOR_BRUSHLESS` (an ESC, one signal
  pin per motor, driven with `Servo::writeMicroseconds()` in the standard
  1000–2000µs RC-PWM range via the ESP32Servo library).
- **`STEERING_MODE`** — *how the robot turns*. This is the one that needed a
  proper explanation:
  - **`STEERING_NONE`** — the robot cannot turn at all; every motor always
    gets the same `throttle` value and `steer` is ignored entirely. Only
    makes sense for something like a single fixed-direction motor with no
    steering mechanism at all (rare — mostly a placeholder/testing mode).
    `drive_do_180()` refuses and prints a message, since there's no way to
    turn.
  - **`STEERING_DIFFERENTIAL`** — "tank steering": there is no separate
    steering actuator. Left-side motors get `throttle - steer`, right-side
    motors get `throttle + steer`, so a positive `steer` slows/reverses the
    left side and speeds up the right side, turning the robot by spinning
    the two sides at different rates. Requires at least 2 motors, each
    tagged `SIDE_LEFT` or `SIDE_RIGHT` in your `MOTOR_LIST` (a 1-motor
    config can't do this — `Drive.h` has a compile-time `#error` that
    catches that combination). `drive_do_180()` spins in place (equal and
    opposite speeds).
  - **`STEERING_SERVO`** — a normal RC-car layout: every drive motor always
    gets the *same* `throttle` (they're not independently steerable), and a
    separate steering servo (`STEERING_SERVO_PIN`) is driven to an angle
    derived from `steer`, centered at `STEERING_SERVO_CENTER_US` (default
    1500µs) ± `STEERING_SERVO_RANGE_US` (default 500µs) at full lock. Works
    with 1, 2, or 4 drive motors. `drive_do_180()` reverses with full
    steering lock.
- **`MOTOR_LIST`** — the `X(name, side, pinA, pinB)` macro list that
  actually generates one `Motor` struct instance per motor (via the X-macro
  pattern — see below) and, for `STEERING_DIFFERENTIAL`, tells `Drive.cpp`
  which physical motors are on which side.

**Combination table** (from `configs/config_template.h`): any
`DRIVE_TOPOLOGY` × `MOTOR_KIND` is valid; `STEERING_MODE` is constrained only
by needing ≥2 motors for `STEERING_DIFFERENTIAL` and a defined
`STEERING_SERVO_PIN` for `STEERING_SERVO` — both checked at compile time with
`#error`, so a bad combination fails the build with a clear message rather
than misbehaving on the bench.

**How the X-macro generates motors.** Your config defines, e.g.:

```cpp
#define MOTOR_LIST \
    X(left,  SIDE_LEFT,  PIN_NONE, PIN_NONE) \
    X(right, SIDE_RIGHT, PIN_NONE, PIN_NONE)
```

`Drive.h` declares `extern Motor motor_##name;` for each entry (by
temporarily `#define X(...)` to that shape, expanding `MOTOR_LIST`, then
`#undef X`), and `Drive.cpp` does the same trick twice more: once to *define*
each `Motor motor_left = {"left", SIDE_LEFT, ...};` instance, and once to
build the `Motor* all_motors[] = { &motor_left, &motor_right };` pointer
table that every loop in `Drive.cpp` (`drive_init`, `motor_write` via index)
actually iterates over. This is the same pattern the Sensors module uses for
its sensor lists — one macro list in config becomes both named globals
(`motor_left`) and a generic array you can loop over, with no per-robot code
duplication in `Drive.cpp` itself.

**Brushed vs. brushless, concretely** (`motor_write()`): brushed motors get
`analogWrite(pinA, speed)` + `analogWrite(pinB, 0)` for forward, swapped for
reverse (classic two-pin H-bridge PWM); brushless motors get a single
`writeMicroseconds()` call mapping -255..255 linearly onto 1000..2000µs
through a `Servo` object per ESC channel, allocated at boot in `drive_init()`.
`state.drive_reversed` (BLE-tunable, `toggle drive_reversed`) flips every
motor's sign centrally in `motor_write()` — handy if the robot ends up
mounted backwards without rewiring.

## Sensors — commands and architecture

**The abstraction.** `src/Sensors/DistanceSensors.h` is the only sensor
header anything outside `src/Sensors/` should include:

```cpp
sensors_init();                  // once, in setup()
sensors_update();                // once per loop tick, before reading anything
float f = dist("front");         // any configured sensor, by name, any type
float g = dist_front();          // shortcut for dist(PRIMARY_FRONT_SENSOR)
float e = sensors_steer_error(); // pre-weighted steering error, feed straight to your PID
```

It doesn't matter whether `"front"` is physically a Sharp IR sensor, a
VL53L0X ToF sensor, or an HC-SR04 ultrasonic — from calling code it's the
same one-line call. `dist(name)` returns `-1` if the name doesn't match any
sensor enabled in your config.

**How a sensor gets its name and behaviour.** Every sensor is one line in an
`X(...)` macro list in your `configs/config_*.h`:

```cpp
#define SHARP_SENSOR_LIST \
    X(left,  PIN_NONE, -45, ROLE_STEER, -1.0f) \
    X(right, PIN_NONE,  45, ROLE_STEER,  1.0f)
```

Format per type (see `configs/config_template.h` section 3 for the exact
column meaning of each):
- `SHARP_SENSOR_LIST`: `X(name, pin, angleDeg, role, weight)`
- `TOF_SENSOR_LIST`: `X(name, xshutPin, i2cAddress, angleDeg, role, weight)`
  — `xshutPin` only matters (must be a real pin, one per sensor, with a
  unique `i2cAddress`) when running more than one VL53L0X on the same I2C
  bus; a single ToF sensor can use `PIN_NONE`.
- `USONIC_SENSOR_LIST`: `X(name, echoPin, trigPin, angleDeg, role, weight)`

Names must be unique across all three lists combined — `dist(name)` searches
across all enabled types.

**`role`** (`ROLE_STEER` / `ROLE_FRONT` / `ROLE_AUX`, from
`src/Utils/HardwareEnums.h`) decides how a sensor is *used*, not how it's
read:
- `ROLE_STEER` — folded into `sensors_steer_error()`'s weighted sum.
- `ROLE_FRONT` — this is the sensor `dist_front()` returns and the one the
  built-in speed-scaling and "too close" escape-maneuver logic in
  `run_line_following()` reacts to. Exactly one sensor should carry this
  role — name it in `PRIMARY_FRONT_SENSOR`.
- `ROLE_AUX` — not automatically used anywhere; read it yourself via
  `dist("name")` in your own logic (e.g. side-facing sensors you want for
  something custom).

**`weight`** is what `sensors_steer_error()` multiplies that sensor's
`value_mm` by before summing across every `ROLE_STEER` sensor:
`error = Σ (weight_i * value_i)`. A left sensor with weight `-1.0` and a
right sensor with weight `+1.0` produces a positive error when the robot is
closer to the right wall than the left, which a standard PID then steers
away from — flip signs/magnitudes to match your physical sensor placement
and desired turning direction. Every sensor's weight is individually
BLE-tunable and saved, named `w_<sensor name>` (e.g. `w_left`) — this is done
by `sensors_register_params()`, called once from `registerParams()`, which
loops every configured sensor and calls `REGISTER_PARAM_NAMED("w_<name>", ...)`
on its weight field. You never register a sensor weight by hand.

**Sensor "commands"**, concretely, are just the generic ParamRegistry
commands applied to `w_<name>`: `get w_left`, `set w_left -1.2`,
`list` (shows every sensor's weight alongside every other tunable). There is
no separate sensor-specific command set — this is intentional, so adding a
sensor never means adding a command.

**Internal architecture, per type.** `src/Sensors/sharp_logic.{h,cpp}`,
`tof_logic.{h,cpp}`, `ultrasonic_logic.{h,cpp}` each follow the same shape: a
struct embedding a `DistReading` (name/role/weight/value_mm/valid, shared
across all types), the type's own `X_LIST` macro expansion generating one
instance + a pointer array per sensor (same X-macro trick as Drive's
`MOTOR_LIST`), and `_init()/_update()/_find()` functions private to that
file. `Sharp` readings are EMA-filtered raw ADC counts (not true mm unless
you add your own linearization curve); `ToF` and ultrasonic readings are real
millimeters. `DistanceSensors.cpp` is the thin aggregator: its
`sensors_init()`/`sensors_update()`/`sensors_register_params()` just call
each enabled type's functions in turn, and `dist(name)` / `find_any()`
linearly searches whichever type-arrays are enabled for a matching name.

**Adding a sensor of an existing type:** one line in the right `X(...)` list
in your config. `dist("that_name")` works immediately — no other file
changes.

**Adding a whole new sensor type** (a camera, a line sensor, a different ToF
chip): copy the shape of `src/Sensors/sharp_logic.h/.cpp` — a struct
embedding `DistReading`, an `X_LIST` config macro, `_init/_update/_find`
functions, a `_register_params()` — and wire your three functions into
`DistanceSensors.cpp`'s three aggregator functions
(`sensors_init`/`sensors_update`/`find_any`/`sensors_register_params`).

## IMU — how to use it, how to get parameters from it

**Only exists when `FEATURE_IMU` is 1** — every declaration in
`src/IMU/IMU_logic.h` is wrapped in `#if FEATURE_IMU`, so referencing an
`imu_*()` function with the feature off is a compile error (deliberately —
it stops you shipping code that silently does nothing on hardware without an
IMU). Wraps a BMI160 6-axis gyro/accelerometer via the FastIMU library, at
I2C address `IMU_I2C_ADDRESS` (set in your config).

**Setup, in order** (already wired up in `Android.ino`/`RobotLogic.h`, shown
here so you know what's actually happening): `Wire.begin(PIN_SDA, PIN_SCL)`
is called once, centrally, in `setup()` — shared with the ToF sensor bus if
you have one, so the IMU module and `tof_logic.cpp` never both try to
initialize I2C. Then `imu_init()` initializes the BMI160 and does a first
calibration pass; if it fails (wrong wiring, wrong address), it prints an
error and every `imu_*()` getter keeps returning stale/zero data —
`imu_ready()` tells you whether init actually succeeded.

**Getting values — one-line calls, called every tick from
`run_debug_logging()`/`run_line_following()`/`Telemetry` already, and free
for you to call from anywhere:**

```cpp
float p  = imu_pitch();        // degrees, complementary-filtered
float y  = imu_yaw();          // degrees, gyro-integrated — drifts, see below
float ax = imu_accel_x_g();    // g's
float ay = imu_accel_y_g();
float az = imu_accel_z_g();
float gx = imu_gyro_x_dps();   // degrees/second
float gy = imu_gyro_y_dps();
float gz = imu_gyro_z_dps();
```

**How pitch is computed** (`imu_update(dt_seconds)`, called once per loop
tick from `service_tick()` whenever `state.imu_enabled` is true): a
complementary filter blends a gyro-integrated angle (responsive, but drifts
over time on its own) with an accelerometer-derived tilt angle (stable
long-term, but noisy tick-to-tick):
`pitch = k * (pitch_prev + gyroY*dt) + (1-k) * pitch_from_accel`, where
`k = state.k_pitch_running` — a BLE-tunable float (`get k_pitch_run` /
`set k_pitch_run 0.95`), closer to 1.0 trusts the gyro more (smoother,
slower to correct drift), closer to 0.0 trusts the accelerometer more
(noisier, no long-term drift).

**Yaw is pure gyro integration** (`yaw += gyroZ * dt`) — there's no
magnetometer on a BMI160, so this drifts unboundedly over time. It's fine for
tracking heading change over a single short run; don't rely on it as an
absolute compass heading over minutes.

**Recalibration:** `imu_recalibrate()` re-runs the BMI160's own
accel/gyro calibration routine (robot must be held still and level) and
resets pitch/yaw to zero. It's called automatically every time you enter
`CALIBRATING` (see the Calibration section) if `state.imu_enabled` is true —
you don't normally need to call it yourself.

**Where it's already used:** `run_line_following()` optionally boosts speed
by 20% while `state.slope_boost` is on and `|pitch| > state.slope_threshold`
(both BLE-tunable) — a simple "give it more power on a visible ramp"
heuristic you can replace with your own use of `imu_pitch()`/`imu_yaw()`.
`run_debug_logging()` can print gyro/accel/yaw/pitch on independent timers
(`log gyro`/`log accel`/`log yaw`/`log pitch` — actually `toggle
log_gyro` etc., since these are just more `ParamRegistry` bools with matching
`log_*_ms` interval params).

## Memory — how it works

**Only exists when `FEATURE_MEMORY` is 1.** `src/Memory/StateStore.cpp`
saves/restores values to the ESP32's flash-backed `Preferences` (NVS) key-value
store. Critically, it does **not** know what any individual parameter is —
it just iterates `ParamRegistry` from position 0 to `count()-1` and reads or
writes each entry generically by its registered name and type:

```cpp
void state_store_init();   // prefs.begin(ROBOT_STORAGE, false) — call once in setup(),
                            // AFTER registerParams() so the registry is already populated
void state_store_load();   // for every registered param that has a saved key in flash,
                            // overwrite the in-memory value with the saved one
void state_store_save();   // write every registered param's CURRENT value to flash
```

This is why `REGISTER_PARAM` alone makes a value persistent with no further
work: `StateStore` automatically picks up every new entry the next time it
runs, because it's reading the same list `get`/`set`/`list` read.

**The NVS namespace** is your config's `ROBOT_STORAGE` string — this is what
lets multiple robots (or multiple sketches) share a flash chip / avoid
collisions; keep it unique per physical robot if you ever flash more than
one config to test on the same board.

**The 15-character key limit:** ESP32 NVS keys are capped at 15 characters.
`Preferences::putFloat(name, ...)` etc. use the param's registered *name*
directly as the NVS key — so any field whose natural name
(`state.escape_trigger_mm` → short name `escape_trigger_mm`, 17 chars) is too
long needs `REGISTER_PARAM_NAMED("esc_trig_mm", ...)` instead of plain
`REGISTER_PARAM`, both to fit the limit and to keep the BLE command short to
type. Several fields in `registerParams()` are already named this way for
exactly that reason (`esc_trig_mm`, `esc_rev_spd`, `esc_turn_spd`,
`esc_time_ms`, `start_delay`, `k_pitch_run`, `slope_thresh`,
`log_dist_ms`, etc.) — copy that pattern for your own long field names.

**When saves/loads actually happen:**
- `state_store_load()` runs once, in `setup()`, right after
  `state_store_init()` — this is what overwrites the compiled-in
  `apply_quick_tunables()` defaults with whatever you last `save`d, if
  anything. If nothing was ever saved (`prefs.isKey(name)` is false for a
  given entry), that entry is left at its just-applied compiled-in default.
- `state_store_save()` runs when `state.save_state` is set to `true` — which
  only happens from the `save` BLE/Serial command
  (`src/BLE/commands.cpp::cmd_save`). `service_tick()` checks that flag every
  tick and calls `state_store_save()` once, then clears it — so saving is
  always an explicit action you trigger, never automatic/continuous (flash
  has a limited write-cycle lifetime, so this is deliberate).

**Resetting to compiled-in defaults without reflashing:** send `defaults`
(see the ParamRegistry/BLE sections) — this re-runs `apply_quick_tunables()`,
which overwrites `state`'s quick-tunable fields with the `tunable::` constants
compiled into `src/Core/RobotLogic.h`, *without* touching flash. If you also
want that reset to survive a reboot, `save` right after.

## Telemetry — does it work, and how to use it

**Yes — it's a working MVP CSV stream, feature-gated behind
`FEATURE_TELEMETRY`.** `src/Telemetry/Telemetry.cpp` prints one CSV line over
`Serial` (115200 baud, the same port used for debug prints and the Serial
command console) every 100ms (10Hz), rate-limited internally so it's safe to
call `telemetry_update()` unconditionally every tick (it's called from
`service_tick()`).

**Format:**
```
T,millis,state,dist_front,steer_error,speed_forward,pitch,yaw
T,123456,RUNNING,182,-4.30,150,1.20,-6.75
```
A header line (`T,millis,state,...`) is printed once, the first time
`telemetry_update()` actually sends a row. Every telemetry row (including the
header) starts with the literal `T,` — that prefix is what lets a receiving
script tell a telemetry row apart from an ordinary `notify()` debug/log line
sharing the same serial stream (you can send both `log dist` and telemetry at
once and a script just filters on lines starting with `T,`). If `FEATURE_IMU`
is off, `pitch`/`yaw` are always printed as `0,0` rather than omitted, so the
column count/order never changes based on other feature flags.

**How to actually consume it:** open the Serial port at 115200 baud from
anything that can read a serial port and split CSV — a Python script with
`pyserial` (`ser.readline()`, `line.startswith("T,")`, `line.split(",")`), the
Arduino IDE's Serial Plotter/Monitor (filter manually), a terminal capture
piped into a spreadsheet, etc. There's no bundled PC-side viewer in this
repo — you bring your own consumer for the CSV.

**What it deliberately is not (yet):** a full ground-station /
bidirectional / radio-based telemetry link. It's Serial-only (so it needs a
USB cable, not standalone over BLE) and one-way. The module is intentionally
isolated — nothing else in the codebase depends on `Telemetry` — specifically
so a future richer transport (BLE notify-based streaming, ESP-NOW to a base
station, etc.) can be added alongside or in place of it without touching any
other module, the same way BLE itself is isolated behind `notify()`.

## How do I…

**…tune something while testing?** Connect over BLE or Serial, send `list`
to see every tunable, `get <name>` / `set <name> <value>` to read/change one,
`save` to persist all of them to flash, `defaults` to snap everything back
to the compiled-in values without reflashing.

**…add a new BLE-tunable number?** Add a field to `RobotState.h` (or, for a
per-sensor value, follow the weight pattern in `sharp_logic.cpp`), then one
line in `src/Core/RobotLogic.h`'s `registerParams()`:
`REGISTER_PARAM(state.my_field);`. Done — it's gettable, settable, listed,
and saved.

**…add a new "factory default" for a quick tunable?** Edit the constant in
the `tunable` namespace near the top of `src/Core/RobotLogic.h`. It takes
effect for new boots (via `apply_quick_tunables()` in `setup()`, unless a
flash-saved value overrides it) and any time someone sends `defaults`.

**…add a new BLE command that's an action, not a number?** In
`src/BLE/commands.cpp`: write a `static void cmd_whatever(char* args) { ... }`,
add `{"whatever", cmd_whatever},` to `command_list[]`. That's it.

**…add a sensor?** Add one line to the right `X(...)` list in your
`configs/config_*.h` (`SHARP_SENSOR_LIST` / `TOF_SENSOR_LIST` /
`USONIC_SENSOR_LIST`). `dist("that_name")` works immediately, no other file
changes. Give it a `weight` if it should feed steering.

**…add a different kind of sensor entirely** (a camera, a line sensor, a
new ToF chip)? Copy the shape of `src/Sensors/sharp_logic.h/.cpp` — a struct
embedding a `DistReading`, an `X_LIST` macro in config, `_init/_update/_find`
functions — and wire it into `DistanceSensors.cpp`'s three aggregator
functions (`sensors_init`, `sensors_update`, `find_any`).

**…build for a new/different robot?** Copy
`configs/config_template.h` to `configs/config_<name>.h`, fill it in, point
`config.h`'s `ACTIVE_ROBOT_CONFIG` at it, upload.

**…change the line-following algorithm itself?** It's one function:
`run_line_following()` in `src/Core/RobotLogic.h`. Everything it calls
(`dist_front()`, `sensors_steer_error()`, `steering_pid.update()`,
`drive_set()`) is a one-line abstraction, so you can restructure the actual
control logic without touching any of `src/Sensors`, `src/drive`, etc.

**…add the future track-mapping feature?** Start from
`src/spaceAverenes/space.h` — it has notes on what it will likely need. It's
empty on purpose; nothing in the rest of the codebase depends on it existing.
