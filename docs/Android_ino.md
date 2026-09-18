# Editing `Android.ino`

This is a guide to `Android.ino` itself: what's already in it, what's safe
to change, and the full toolbox of functions it can call into. For *what a
worked-example algorithm looks like* see [Algorithm.md](Algorithm.md); for
*how the codebase as a whole fits together* see
[Architecture](Arhitecture.md). This doc is the one to read before you
start editing the file.

## What belongs here vs. in `config.h` / `Defaults.h`

`config.h` describes the robot you built - pins, addresses, angles, which
features are on. `Defaults.h` holds starting tuning values and calibration
constants you rarely need to touch. `Android.ino` is the robot's
*behaviour* - the state machine, and (in `onRunning()`) whatever driving
algorithm you write. A rule of thumb: if a change is "my robot has
different hardware," it's `config.h`; if it's "I want a different starting
number," it's `Defaults.h`; if it's "my robot should act differently," it's
`Android.ino`.

You should basically never need a fourth file - everything under `src/` is
plumbing that `Android.ino` calls into, and you only need to open it if
you're adding a genuinely new capability (see "How to extend this" in
[Architecture](Arhitecture.md)).

## What's already in the file

- **`onRunning()`** - intentionally blank. This is where you write your own
  sensor reading, steering algorithm and drive calls - see below and
  [Algorithm.md](Algorithm.md) for a worked example to study or paste in.
- **One function per other state**: `onIdle()`, `onReady()`, `onCountdown()`,
  `onCalibration()`, `onForward()`, `onBackwards()`.
- **`setup()`** - brings up whichever features are enabled, in the right
  order (load saved state, then motors/servo, then sensors, then IMU, then
  BLE).
- **`loop()`** - updates the delta-time clock, updates the IMU if enabled,
  switches on `state.mode` to call exactly one state function, then
  services BLE debug logging and any pending "save" request.

That's the whole file. Nothing else needs to exist here.

## Writing `onRunning()`

`onRunning()` is called once per control-loop tick while `state.mode ==
Mode::RUNNING`, and is handed `dtSeconds` (seconds since the last tick). It
starts empty on purpose - three things go here, in order:

1. **Read your sensors.** `sensor_read(name)` returns millimeters for the
   sensor with that name (whatever you called it in `config.h`), or `-1` if
   it's not configured:
   ```cpp
   int16_t frontDist = sensor_read("front");
   int16_t leftDist  = sensor_read("left");
   int16_t rightDist = sensor_read("right");
   ```
2. **Decide on a steering correction and a forward speed.** This is your
   algorithm - a PID loop, simple if/else rules, anything you want. A
   `PIDController` called `steeringPID` is already declared above
   `onRunning()` if you want one:
   ```cpp
   float steer = steeringPID.update(error, dtSeconds); // positive = turn right
   ```
   Remember to call `steeringPID.configure(kp, ki, kd)` (already done once
   per tick in `loop()`, right before `onRunning()` is called) and
   `steeringPID.reset()` when starting a fresh run (already done in
   `onCountdown()`).
3. **Drive.**
   ```cpp
   drive_apply(forwardSpeed, steer);
   ```
   Handles servo vs. differential steering, speed ramping, and
   `drive_reversed` for you - you never need to touch individual motors.

See [Algorithm.md](Algorithm.md) for a complete, working example (PID
wall-following) that puts these three steps together - copy it in wholesale
as a starting point if you don't want to start from a blank page.

## Things you can change without reading further

- **Tune a number** - don't edit the file at all, use the BLE console (see
  [BLE_Commands.md](BLE_Commands.md)) or change the `DEFAULT_*` value in
  `Defaults.h`.
- **Change what any other state does** - `onIdle()`, `onReady()`,
  `onCountdown()`, `onCalibration()`, `onForward()`, `onBackwards()` are
  all short, plain functions. Edit the one you want to change; the others
  (and `loop()`'s dispatch) don't need to know anything happened.

## Adding a new state

The state machine is driven by the `Mode` enum in
`src/Utils/RobotState.h` - `Android.ino` doesn't define it, so a genuinely
new state (not just a variant of an existing one) means touching that file
too:

1. Add the new value to `enum class Mode` in `src/Utils/RobotState.h`.
2. Write its handler function in `Android.ino`, next to the others.
3. Add a `case Mode::YourState: onYourState(); break;` to the `switch` in
   `loop()`.
4. If BLE commands should be able to enter/leave it, add or update a
   `STATE_BIT(Mode::YourState)` entry in `src/BLE/commands.cpp`'s
   `command_list[]` - see [BLE_Commands.md](BLE_Commands.md). `forward`/
   `backward`/`stop` and the `FORWARD`/`BACKWARDS` states are a small
   worked example of exactly this if you want a template to copy.

## Adding a new tunable value

If your change needs a new number that should be BLE-tunable and
persisted, don't just declare a local/global in `Android.ino`:

1. Add a field to `struct RobotState` in `src/Utils/RobotState.h`, with a
   `DEFAULT_*` fallback pulled from `Defaults.h`.
2. Add a `DEFAULT_*` value in `Defaults.h`.
3. Add a BLE command (or extend an existing one, like `k`/`speed`/`dist`)
   in `src/BLE/commands.cpp` to read/set it.
4. If it should survive a reboot, add a `get`/`put` pair for it in
   `src/Memory/Memory.cpp`.

Then just read `state.your_field` from `Android.ino` like any other value.

## The toolbox - what you can call from `Android.ino`

Everything below is available once the `#include`s at the top of the file
have run (`Libraries.h` is the one that pulls in every driver your
`config.h` selections need). Anything gated by `#if` is only declared when
that feature is on in `config.h` - guard your own calls to it the same way,
exactly like the existing code does for `Is_IMU`/`Is_blueTooth`.

**State** (`src/Utils/RobotState.h`)
- `state` - the one global `RobotState`. Every tunable and runtime value
  lives on it (`state.mode`, `state.pid.kp`, `state.speed_forward`, ...) -
  see the struct itself for the full field list.
- `Mode` - the state enum (`Mode::IDLE`, `Mode::READY`, ...).

**Sensors** (`src/Sensors/Sensors.h`)
- `sensor_read(const char* name)` - distance in mm for the sensor with this
  name, across whichever technologies (TOF/Sharp/Ultrasonic) are enabled.
  Returns `-1` if nothing by that name is configured. This is the only
  sensor call you should need from `Android.ino`.

**Drive** (`src/drive/Drive.h`)
- `drive_apply(int forwardSpeed, float steer)` - the one call that actually
  moves the robot. Handles servo vs. differential steering, speed ramping,
  and `drive_reversed` for you.
- `drive_reset()` - stops the robot and clears the speed ramp/maneuver.
  Call this on entry to every non-driving state (see `onIdle`/`onReady`/
  `onCountdown` for the pattern).
- `maneuver_active()` / `maneuver_service()` - is a timed maneuver (reverse
  + turn) in progress, and drive it one tick further. Handy if your
  algorithm wants a non-blocking "back up and turn" sequence, the way the
  worked example in [Algorithm.md](Algorithm.md) uses it.
- `maneuver_start_reverse_turn(int16_t leftDist, int16_t rightDist)` -
  begin a reverse+turn, picking a direction from whichever side has more
  room.
- `maneuver_start_180()` - begin a longer, in-place turn.

**IMU** (`src/IMU/IMU_logic.h`, only if `Is_IMU`)
- `imu_update(float dtSeconds)` - already called once per `loop()` tick;
  you don't need to call this again.
- `imu_pitch()`, `imu_yaw()`, `imu_accel_x/y/z()`, `imu_gyro_x/y/z()` - most
  recent readings as of the last `imu_update()`.

**BLE** (`src/BLE/RobotBLE.h`, `src/BLE/Telemetry.h`, only if `Is_blueTooth`)
- `notify(const char* fmt, ...)` - printf-style, sends a line over BLE.
  Safe to call even with nothing connected.
- `service_debug_logs()` - already called once per `loop()` tick.

**Memory** (`src/Memory/Memory.h`, only if `Memory`)
- `load_state()` / `save_state()` - already called from `setup()` and
  `loop()` respectively; you shouldn't need to call these directly.

**Calibration** (`src/Calibrations/Calibration.h`)
- `run_calibration()` - already called from `onCalibration()`.

**Timing / math**
- `dt` - the shared `DeltaTime`; `dtSeconds` is passed into `onRunning()`
  already, you don't need to touch `dt` directly.
- `steeringPID` - a `PIDController` instance, ready to use if your
  algorithm wants a PID loop. If you don't want one, ignore or remove it.
- Arduino's own `millis()`, `constrain()`, `map()`, `min()`, `max()` are
  all available as usual.

## What NOT to put in `Android.ino`

- **Sensor/motor drivers.** If you're wiring up a new distance sensor
  *technology* (not just a new sensor of an existing type - that's a
  `config.h` line, see [Setup.md](Setup.md)), that driver lives under
  `src/Sensors/`, not here. See "How to extend this" in
  [Architecture](Arhitecture.md).
- **Pins, addresses, angles, feature flags.** All of that is `config.h`.
  `Android.ino` should have no pin numbers in it at all.
- **Starting/default values.** Those are `Defaults.h`, not a hardcoded
  number here.
- **BLE command definitions.** New commands go in
  `src/BLE/commands.cpp`'s `command_list[]`, not as ad-hoc parsing in
  `Android.ino`.
