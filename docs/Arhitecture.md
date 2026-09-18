# Architecture

This document explains how the codebase fits together: what lives where, why
it's split the way it is, and — most importantly — exactly what to touch for
each kind of change you'll want to make. If you only read one section, read
["How do I…"](#how-do-i) below.

## The three layers

```
configs/config_<robot>.h   <- WHAT you have: pins, sensors, motors, features
config.h                   <- WHICH config is active right now (one line)
Android.ino                <- the state machine + values you tune a lot
src/*                      <- HOW everything works, so Android.ino doesn't have to
```

You will spend almost all of your time in the top two layers. `src/` exists
so you don't have to.

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
| State | `src/Utils/RobotState.h/.cpp` | The single `RobotState state` struct everything reads/writes. No logic. |
| Params | `src/Utils/ParamRegistry.h/.cpp` | Name → pointer registry. Makes a value BLE-tunable + saveable. |
| PID | `src/Utils/PIDAlgorithm.h` | Generic PID loop, reads gains live from `state.pid`. |
| Sensors | `src/Sensors/*` | `dist(name)`, `dist_front()`, `sensors_steer_error()`. Hides Sharp/ToF/ultrasonic differences. |
| IMU | `src/IMU/IMU_logic.h/.cpp` | `imu_pitch()`, `imu_yaw()`, accel/gyro getters. |
| Drive | `src/drive/Drive.h/.cpp` | `drive_set(throttle, steer)`. Hides 1/2/4-motor, brushed/brushless, servo/differential steering. |
| Calibration | `src/Calibrations/Calibration.h/.cpp` | Live sensor dump while `RunningState::CALIBRATING`. |
| Memory | `src/Memory/StateStore.h/.cpp` | Saves/restores every `ParamRegistry` entry to flash (`Preferences`). |
| Telemetry | `src/Telemetry/Telemetry.h/.cpp` | CSV stream to a PC over Serial. |
| Space awareness | `src/spaceAverenes/space.h/.cpp` | **Not implemented.** Scaffolding + notes for a future track-mapping feature. |
| BLE | `src/BLE/RobotBLE.h/.cpp`, `src/BLE/commands.h/.cpp` | Everything Bluetooth. The only module that knows BLE exists. |

Every module above is included from `Android.ino` behind its feature's
`#if FEATURE_*` (or `#if SENSOR_*_ENABLED`), so code for a feature you don't
use doesn't even get compiled in.

## The parameter registry — why "easy to add a BLE command" mostly isn't a command

The single biggest source of boilerplate in the old firmware was: add a
field to the state struct, add a case to a giant BLE command switch to read
it, add another case to set it, add lines to save it to flash, add lines to
load it back. Four places, every time, for what is conceptually one idea
("this number should be tunable").

`ParamRegistry` (`src/Utils/ParamRegistry.h`) collapses that into one line:

```cpp
REGISTER_PARAM(state.pid.kp);
```

called once from `Android.ino`'s `registerParams()`. That single line makes
`kp`:

- readable over BLE: `get kp`
- settable over BLE: `set kp 1.4`
- flippable if it's a bool: `toggle scaled_speed`
- listed by: `list`
- saved to flash and restored on boot automatically, via `src/Memory/StateStore.cpp`,
  which just iterates the registry — it doesn't know `kp` exists any more
  than `commands.cpp` does.

`REGISTER_PARAM_NAMED("short_name", state.some_long_field_name)` does the
same but with an explicit BLE-visible name — use it when the field's real
name would be longer than 15 characters (the ESP32 flash storage key limit)
or when you just want a shorter name on the wire.

Sensor weights follow the same idea but are registered by the Sensors module
itself (`sensors_register_params()`), since they're generated from your
`configs/config_*.h` sensor lists rather than being fixed fields — see
`src/Sensors/sharp_logic.cpp` for how.

You still write a real *command* (an entry in `src/BLE/commands.cpp`'s
`command_list[]`) for things that aren't just a number — `start`, `stop`,
`calibrate`, `180`, and so on. See ["Add a new BLE command"](#add-a-new-ble-command-thats-an-action-not-a-number)
below.

## The sensor abstraction

`src/Sensors/DistanceSensors.h` is the only sensor header `Android.ino`
includes. Everything else (`sharp_logic`, `tof_logic`, `ultrasonic_logic`) is
an internal implementation detail it hides behind three calls:

```cpp
sensors_update();              // once per loop tick
float f = dist("front");       // any named sensor, any type
float e = sensors_steer_error(); // pre-weighted sum, ready for your PID
```

Each entry in a `configs/config_*.h` sensor list (`SHARP_SENSOR_LIST`,
`TOF_SENSOR_LIST`, `USONIC_SENSOR_LIST`) carries a `role`:

- `ROLE_STEER` — included in `sensors_steer_error()`'s weighted sum
- `ROLE_FRONT` — used by `dist_front()` / the built-in speed-scaling and
  escape-maneuver logic (exactly one sensor should have this role — the one
  named in `PRIMARY_FRONT_SENSOR`)
- `ROLE_AUX` — read via `dist("name")` in your own code, otherwise ignored

and a `weight`, which is what `sensors_steer_error()` multiplies that
sensor's reading by. Weights are individually BLE-tunable (`w_<name>`) —
see the Params section above.

This is the mechanism behind "type a simple command to get the distance for
one of the sensors and integrate it into your logic": add a line to a
sensor list in your config, and `dist("that_name")` works, regardless of
whether it's a Sharp IR, a VL53L0X, or an ultrasonic sensor underneath.

## The drive abstraction

`src/drive/Drive.h` exposes exactly this:

```cpp
drive_set(throttle, steer);  // -255..255 each
drive_stop();
drive_do_180();
```

What that *means* depends entirely on your config's `DRIVE_TOPOLOGY` /
`STEERING_MODE` / `MOTOR_KIND` / `MOTOR_LIST` (see
`configs/config_template.h` section 5 for the full combination table: 1/2/4
motors, brushed or brushless, differential or servo steering). Your
`RUNNING`-state logic in `Android.ino` never needs to know which — it always
just calls `drive_set(speed, steer)`.

## The state machine

`RobotState::running_state` (`src/Utils/RobotState.h`) is one of:

```
IDLE -> READY -> COUNTDOWN -> RUNNING
  ^        |
  +--- CALIBRATING
MANUAL_FORWARD / MANUAL_BACKWARD  (debug driving, entered from anywhere via BLE)
```

`Android.ino`'s `loop()` does three things every tick, in order:
1. Service sensors/IMU/telemetry/memory-save (feature-gated, always safe to call).
2. If `running_state` just changed, call `on_state_enter()` once (e.g. kick
   off calibration, reset the PID's integral term).
3. Run the current state's per-tick behaviour in a `switch`.

Commands (`src/BLE/commands.cpp`) only ever *set* `state.running_state`.
They never contain driving logic themselves — that keeps "what can happen"
(the switch in `loop()`) and "what triggers it" (BLE commands) cleanly
separated.

## BLE — how it stays out of everything else

`src/BLE/RobotBLE.cpp` is the *only* file that includes `<BLEDevice.h>` and
friends. Two things make that possible:

1. **Incoming**: BLE writes are parsed into `command` + `args` and hand off
   to `handle_command()` in `commands.cpp` — the same function a Serial
   console or any other transport could call.
2. **Outgoing**: every module calls `notify(fmt, ...)`
   (`src/Utils/Notify.h`), never BLE directly. `notify()` always echoes to
   Serial; `RobotBLE.cpp` additionally registers itself as an extra output
   sink at boot (`notify_set_sink(...)`) so the same call also goes out over
   BLE when it's enabled. Nothing else needs an `#if FEATURE_BLUETOOTH` guard
   around a print statement.

If you ever add a second transport (say, ESP-NOW to a ground station), it
plugs into exactly these two points and nothing else in the codebase changes.

## How do I…

**…tune something while testing?** Connect over BLE, send `list` to see
every tunable, `get <name>` / `set <name> <value>` to read/change one,
`save` to persist all of them to flash.

**…add a new BLE-tunable number?** Add a field to `RobotState.h` (or, for a
per-sensor value, follow the weight pattern in `sharp_logic.cpp`), then one
line in `Android.ino`'s `registerParams()`: `REGISTER_PARAM(state.my_field);`.
Done — it's gettable, settable, listed, and saved.

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
`run_line_following()` near the bottom of `Android.ino`. Everything it calls
(`dist_front()`, `sensors_steer_error()`, `steering_pid.update()`,
`drive_set()`) is a one-line abstraction, so you can restructure the actual
control logic without touching any of `src/`.

**…add the future track-mapping feature?** Start from
`src/spaceAverenes/space.h` — it has notes on what it will likely need.
