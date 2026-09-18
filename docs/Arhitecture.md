# Architecture

## File map

```
config.h            Hardware description (you edit this)
Defaults.h           Starting tuning values + calibration constants (rarely edited)
Android.ino          State machine + driving algorithm (you edit this - onRunning() is blank on purpose)

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
   ▲  │                                                     │
   │  │ forward/backward         stop                       │
   │  ▼                                                      │
   │ FORWARD / BACKWARDS ◄──────────────────────────────────┘
   │  │
   └──┘ stop
   ▲
   │ calibrate           (runs once, then back to IDLE)
   └──────────────────────────── CALIBRATION
```

`FORWARD`/`BACKWARDS` are manual bench-test states - reachable from `IDLE`
or `READY` via the BLE `forward`/`backward` commands, and left the same way
`RUNNING` is, via `stop`. They just drive straight at `speed_forward`/
`speed_reverse`, useful for checking your wiring before your algorithm is
ready.

`state.mode` (a `Mode` enum, in `src/Utils/RobotState.h`) is the single
source of truth for which state the robot is in. `Android.ino`'s `loop()`
switches on it and calls exactly one function per state - see
`docs/Algorithm.md` for a worked example of what `onRunning()` could do (it
ships blank), the others are short enough to just read directly in
`Android.ino`.

Commands over BLE (`src/BLE/commands.cpp`) are only allowed in certain
states - e.g. `start` only works from `READY`, `stop` works from
`COUNTDOWN`/`RUNNING`/`FORWARD`/`BACKWARDS`. See
[BLE_Commands.md](BLE_Commands.md).

## `RobotState` - the one shared state struct

Every tunable parameter and every piece of runtime state (current mode,
PID gains, speeds, distances, IMU flags, debug/logging flags, the
in-progress maneuver) lives in a single `RobotState state;` global, defined
in `Android.ino` and declared `extern` in `src/Utils/RobotState.h`. This is
what makes the BLE console, `Memory` (save/load) and whatever driving
algorithm you write all agree on what a value is called and means - there's
exactly one `state.pid.kp`, not a copy per module.

Its fields default to the `DEFAULT_*` macros from `Defaults.h`. `load_state()`
(if `Memory` is on) then overwrites any of them that were previously saved,
falling back to that same default for anything that wasn't.

## The sensor/motor config-array pattern

`config.h` declares sensors and motors as plain, single-declaration arrays,
one line per device, e.g.:

```cpp
static const struct { const char* name; uint8_t xshutPin; uint8_t i2cAddress; int16_t angle; } TOF_SENSORS[] = {
  { "front", 4, 0x30, 0 },
};
```

The `static const struct {...}` part is boilerplate you never need to
touch or understand - it just says "here's a list of TOF sensors, each one
a name/pin/address/angle." Everything you actually edit is inside the `{ }`
list below it, one line per device.

Each driver module (`tof_logic`, `sharp_logic`, `ultrasonic_logic`,
`Drive`) copies that list into its own runtime array of full sensor/motor
objects during its `*_setup()` function, then offers a `..._getByName("name")`
lookup over that array. There's no macro magic involved: to add or remove a
sensor or motor, add or remove one line from the matching array in
`config.h` - no driver code needs to change. If you're not modifying a
driver, you never need to understand this beyond "one line per device, in
`config.h`".

## How to extend this

**Add a new BLE command:** add an entry to `command_list[]` in
`src/BLE/commands.cpp` - see the existing entries and
[BLE_Commands.md](BLE_Commands.md).

**Add a new distance sensor technology:** copy the shape of
`src/Sensors/sharp_logic.h`/`.cpp` (setup, read-by-pointer, read-by-name,
get-by-name), add a `..._getByName`/`..._read` call to `sensor_read()` in
`src/Sensors/Sensors.cpp`, and add a matching config-array (see above) and
feature flag to `config.h`.

**Write or change the driving algorithm:** `onRunning()` in `Android.ino`
ships blank - see [Android_ino.md](Android_ino.md) for the full toolbox of
functions you can call, and [Algorithm.md](Algorithm.md) for a complete
worked example (PID wall-following) you can study or paste in as a starting
point. Nothing else needs to change; `drive_apply()` and the sensor/IMU
layers underneath don't care how you computed the speed/steer you hand
them.

**Add a new tunable parameter:** add a field to `RobotState` (with a
`DEFAULT_*` fallback from `Defaults.h`), a BLE command to read/set it in
`commands.cpp`, and a `get`/`put` pair in `src/Memory/Memory.cpp` if it
should be saved.
