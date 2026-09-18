# Architecture

## File map

```
config.h            Hardware description + starting tuning values (you edit this)
Android.ino          State machine + driving algorithm (you edit this)

src/drive/           Motor PWM, servo steering, speed ramping, the reverse+turn maneuver
src/Sensors/         TOF / Sharp / ultrasonic drivers + sensor_read("name") dispatcher
src/IMU/              IMU driver (FastIMU) + pitch/yaw estimation
src/BLE/              BLE connection, the tuning-console command table, periodic telemetry
src/Memory/          Save/load tunable parameters to flash (ESP32 Preferences)
src/Calibrations/    The one-shot calibration routine run from Mode::CALIBRATION
src/Utils/            Shared plumbing: RobotState (all runtime state), PID controller,
                      delta-time helper, and Libraries.h which wires everything together
src/spaceAverenes/   Stub for a not-yet-implemented feature (spaceControl) - see config.h
```

`src/Utils/Libraries.h` is included once, at the top of `Android.ino`. It
runs the `config.h` sanity checks (the `#error`/`#warning` messages you see
if something's misconfigured) and then includes every module whose feature
flag is on, so `Android.ino` itself doesn't need a long list of `#include`s.

## The state machine

```
        ready              start           dist_reverse
 IDLE ────────► READY ────────────► COUNTDOWN ────────► RUNNING
   ▲                                                        │
   │                    stop                                │
   └────────────────────────────────────────────────────────┘
   ▲
   │ calibrate           (runs once, then back to IDLE)
   └──────────────────────────── CALIBRATION
```

`state.mode` (a `Mode` enum, in `src/Utils/RobotState.h`) is the single
source of truth for which state the robot is in. `Android.ino`'s `loop()`
switches on it and calls exactly one function per state - see
`docs/Algorithm.md` for what `onRunning()` does, the others are short enough
to just read directly in `Android.ino`.

Commands over BLE (`src/BLE/commands.cpp`) are only allowed in certain
states - e.g. `start` only works from `READY`, `stop` only from `COUNTDOWN`
or `RUNNING`. See [BLE_Commands.md](BLE_Commands.md).

## `RobotState` - the one shared state struct

Every tunable parameter and every piece of runtime state (current mode,
PID gains, speeds, distances, IMU flags, debug/logging flags, the
in-progress maneuver) lives in a single `RobotState state;` global, defined
in `Android.ino` and declared `extern` in `src/Utils/RobotState.h`. This is
what makes the BLE console, `Memory` (save/load) and the driving algorithm
all agree on what a value is called and means - there's exactly one
`state.pid.kp`, not a copy per module.

Its fields default to the `DEFAULT_*` macros from `config.h`. `load_state()`
(if `Memory` is on) then overwrites any of them that were previously saved,
falling back to that same default for anything that wasn't.

## The `X(...)` sensor/motor list pattern

`config.h` declares sensors and motors as lists like:

```cpp
#define TOF_SENSOR_LIST \
  X(front, 4, 0x30, 0)
```

Each driver module then expands that list twice with a local `#define X(...)`
- once to declare/define one struct instance per entry, once to build a
pointer array of all of them - then `#undef X`. This is a plain C
preprocessor trick (no library involved): it lets you add or remove a
sensor by editing one line in `config.h`, without touching any driver code.
If you're not modifying a driver, you never need to understand this beyond
"one line per sensor, in `config.h`".

## How to extend this

**Add a new BLE command:** add an entry to `command_list[]` in
`src/BLE/commands.cpp` - see the existing entries and
[BLE_Commands.md](BLE_Commands.md).

**Add a new distance sensor technology:** copy the shape of
`src/Sensors/sharp_logic.h`/`.cpp` (setup, read-by-pointer, read-by-name,
get-by-name), add a `..._getByName`/`..._read` call to `sensor_read()` in
`src/Sensors/Sensors.cpp`, and add the matching `X(...)` list format and
flag to `config.h`.

**Change the driving algorithm entirely:** edit `onRunning()` and its two
helpers in `Android.ino` - see [Algorithm.md](Algorithm.md). Nothing else
needs to change; `drive_apply()` and the sensor/IMU layers underneath don't
care how you computed the speed/steer you hand them.

**Add a new tunable parameter:** add a field to `RobotState` (with a
`DEFAULT_*` fallback from `config.h`), a BLE command to read/set it in
`commands.cpp`, and a `get`/`put` pair in `src/Memory/Memory.cpp` if it
should be saved.
