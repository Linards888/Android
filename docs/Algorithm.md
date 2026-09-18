# Algorithm (a worked example)

`onRunning()` in `Android.ino` ships **blank** - you write your own driving
algorithm there (see [Android_ino.md](Android_ino.md)). This page walks
through one complete, working example - PID wall-following with a
non-blocking dead-end reverse+turn - step by step, so you have something
concrete to study, adapt, or paste in wholesale as your own starting point
(the exact code is at the bottom). Nothing here is currently running unless
you put it there yourself.

## Sensor naming

`sensor_read(name)` looks up a sensor by name across whichever sensor
technologies are enabled in `config.h` (TOF, Sharp, ultrasonic - mix and
match freely) and returns its distance in millimeters, or `-1` if no sensor
with that name exists. The algorithm below only ever asks for three names:

- `"front"` - facing the direction of travel
- `"left"` / `"right"` - facing left/right, or angled forward-left/forward-right

Whatever sensor(s) you actually wired up, name them `front`/`left`/`right`
in `config.h`'s sensor lists and the algorithm picks them up automatically.
Unnamed or differently-named sensors are still readable by hand, just not
used here.

## Step by step

**1. In-progress maneuver takes priority.**
If a reverse+turn maneuver is already running (see below), or the manual
`180` debug command was just triggered over BLE, that maneuver keeps
driving the motors this tick and everything else is skipped.

**2. Read the sensors.**
```cpp
int16_t frontDist = sensor_read("front");
int16_t leftDist  = sensor_read("left");
int16_t rightDist = sensor_read("right");
```

**3. Dead end ahead?**
If `frontDist` is valid and closer than `state.dist_reverse` (Defaults.h
`DEFAULT_DIST_REVERSE`, mm), the robot hands off to the reverse+turn
maneuver instead of continuing to steer normally (see below).

**4. Work out the steering error** (`computeSteeringError`):

| Sensors configured | Error formula | Meaning |
| --- | --- | --- |
| Both `left` and `right` | `right * k_right_side - left * k_left_side` | Drive down the middle: steer toward whichever side reads farther away. |
| Only `left` | `(dist_far - left) * k_left` | Hold `dist_far` mm away from that one wall. |
| Only `right` | `(right - dist_far) * k_right` | Same idea, mirrored. |
| Neither | `0` | Drive straight. |

The result is clamped to `+-state.dist_constrain` so one bad/out-of-range
reading can't throw a huge, sudden correction at the PID.

**5. Run the error through the steering PID.**
`steeringPID.update(error, dtSeconds)` turns the error into a signed
steering correction (positive = turn right). Gains are `state.pid.kp/ki/kd`
- tune these live over BLE with the `k p`/`k i`/`k d` commands (see
[BLE_Commands.md](BLE_Commands.md)).

**6. Optional IMU steering nudge.**
If `Is_IMU` and `state.imu_enabled` are both on, a small correction from
lateral acceleration is added: `steer += imu_accel_x() * state.k_accel_nudge`.
`k_accel_nudge` starts at `0`, so this does nothing until you deliberately
tune it in.

**7. Work out forward speed** (`computeForwardSpeed`):
Starts at `state.speed_forward`. If `Is_IMU`, `state.imu_enabled` and
`state.slope_boost` are all on and the measured pitch exceeds
`state.slope_threshold` (degrees), extra speed is added:
`speed += k_pitch_running * pitch`. The result is clamped to
`state.speed_min..state.speed_max`.

**8. Drive.**
`drive_apply(forwardSpeed, steer)` (in `src/drive/Drive.cpp`) turns that
into actual motor/servo commands: it ramps speed changes using
`state.accel`/`state.brake` so they don't jump instantly, flips everything
if `state.drive_reversed` is set, and either differential-steers two/four
motors or drives one motor plus a servo, depending on your `config.h` drive
layout.

## The reverse+turn maneuver

When the front sensor sees a dead end, the robot runs a short, timed,
**non-blocking** sequence (no `delay()`, so BLE and sensor reads keep
working throughout):

1. **Reversing** - backs up at `speed_reverse * k_reverse` for
   `MANEUVER_REVERSE_MS`, then
2. **Turning** - pivots in place (one side forward, the other reverse, or
   the servo hard over) for `MANEUVER_TURN_MS`, toward whichever side's
   sensor read more open space (or the last-used direction, if both/neither
   side sensor is configured), then
3. hands control back to the normal steering logic above.

The manual `180` BLE command runs the same turning phase, but for the
longer `MANEUVER_180_MS` and without the reversing step first - useful for
manually spinning the robot around on the bench.

All three durations are in `Defaults.h`.

## Copy-paste starting point

If you'd rather start from this than a blank page, paste the following
into `Android.ino`, replacing the empty `onRunning()` there (and adding the
two helpers above it, the same way the comments in `Android.ino` suggest):

```cpp
static int computeSteeringError(int16_t leftDist, int16_t rightDist) {
  int error = 0;

  if (leftDist >= 0 && rightDist >= 0) {
    error = (int)(rightDist * state.pid.k_right_side - leftDist * state.pid.k_left_side);
  } else if (leftDist >= 0) {
    error = (int)((state.dist_far - leftDist) * state.pid.k_left);
  } else if (rightDist >= 0) {
    error = (int)((rightDist - state.dist_far) * state.pid.k_right);
  }

  return constrain(error, -state.dist_constrain, state.dist_constrain);
}

static int computeForwardSpeed() {
  int speed = state.speed_forward;

#if Is_IMU
  if (state.imu_enabled && state.slope_boost && imu_pitch() > state.slope_threshold) {
    speed += (int)(state.k_pitch_running * imu_pitch());
  }
#endif

  return constrain(speed, state.speed_min, state.speed_max);
}

static void onRunning(float dtSeconds) {
  if (maneuver_active()) {
    maneuver_service();
    return;
  }
  if (state.debug.do_manual_180) {
    state.debug.do_manual_180 = false;
    maneuver_start_180();
    maneuver_service();
    return;
  }

  int16_t frontDist = sensor_read("front");
  int16_t leftDist  = sensor_read("left");
  int16_t rightDist = sensor_read("right");

  if (frontDist >= 0 && frontDist <= state.dist_reverse) {
    maneuver_start_reverse_turn(leftDist, rightDist);
    maneuver_service();
    return;
  }

  int error = computeSteeringError(leftDist, rightDist);
  float steer = steeringPID.update((float)error, dtSeconds);

#if Is_IMU
  if (state.imu_enabled) {
    steer += imu_accel_x() * state.k_accel_nudge;
  }
#endif

  drive_apply(computeForwardSpeed(), steer);
}
```

This is exactly the algorithm the steps above describe - use it as-is,
tune it over BLE, or use it as a reference while writing something of your
own.
