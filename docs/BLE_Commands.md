# BLE Commands

Only available if `Is_blueTooth` is on in `config.h`. Connect with any
generic BLE serial/terminal app - the robot advertises as `BLE_DEVICE_NAME`
(default `Folkrace`). Send commands as plain text; most reply with the
current value(s) whether you're reading or setting them.

Every command is only allowed in certain states - sending one at the wrong
time gets `not allowed in this mode` back instead of running it. That's
listed as **Allowed in** below.

## State control

| Command | Allowed in | Does |
| --- | --- | --- |
| `ready` | Idle | Arms the robot: `Idle` -> `Ready`. |
| `start` | Ready | Begins the countdown: `Ready` -> `Countdown` -> (after `start_delay_ms`) `Running`. |
| `stop` | Countdown, Running | Stops immediately, back to `Idle`. |
| `calibrate` | Idle | Runs the calibration routine once, then back to `Idle`. See [Calibration.md](Calibration.md). |
| `state` | any | Prints a summary: mode, PID gains, distances, speeds, IMU flags. |
| `help` | any | Lists every command name. |

## Tuning - `k` (PID and steering gains)

`k` alone reports every gain. `k <name> [value]` reports (no value given)
or sets (value given) one gain:

| Name | Field | Meaning |
| --- | --- | --- |
| `p` / `i` / `d` | `pid.kp/ki/kd` | Steering PID gains - see [Algorithm.md](Algorithm.md). |
| `left` / `right` | `pid.k_left/k_right` | Weight when following a single wall with only that one side sensor configured. |
| `left_side` / `right_side` | `pid.k_left_side/k_right_side` | Weight on each side sensor when both are configured (driving down the middle). |
| `reverse` | `k_reverse` | Speed multiplier while backing away from a dead end. |
| `pitch_running` | `k_pitch_running` | Extra forward speed per degree of pitch, if `slope_boost` is on (needs `Is_IMU`). |
| `accel_nudge` | `k_accel_nudge` | Steering correction per unit of lateral acceleration (needs `Is_IMU`). |

Example: `k p 2.5` sets Kp to 2.5 and echoes it back.

## Tuning - `speed`

`speed` alone reports all four. `speed <name> [value]`:

| Name | Field |
| --- | --- |
| `forward` | `speed_forward` |
| `reverse` | `speed_reverse` |
| `min` | `speed_min` |
| `max` | `speed_max` |

## Tuning - `dist` (millimeters)

`dist` alone reports all four. `dist <name> [value]`:

| Name | Field | Meaning |
| --- | --- | --- |
| `near` | `dist_near` | Reserved - not currently used by the built-in algorithm. |
| `far` | `dist_far` | Target distance when following a single wall. |
| `reverse` | `dist_reverse` | Front distance that triggers the reverse+turn maneuver. |
| `constrain` | `dist_constrain` | Clamp on the steering error fed into the PID. |

## Other tuning

| Command | Meaning |
| --- | --- |
| `delay [ms]` | The `Countdown` length after `start`, before `Running` begins. |
| `accel [step]` | Max PWM increase per control-loop tick while speeding up. |
| `brake [step]` | Max PWM decrease per control-loop tick while slowing down. |
| `slope_t [degrees]` | Pitch threshold (needs `Is_IMU`) above which the robot is considered "on a ramp". |

## Toggles (no argument - each call flips the flag)

| Command | Flips | Notes |
| --- | --- | --- |
| `reverse_drive` | `drive_reversed` | Use if the motors are wired backwards instead of re-wiring them. |
| `slope_boost` | `slope_boost` | Enables the uphill speed boost (needs `Is_IMU`). |
| `imu` | `imu_enabled` | Enables IMU-assisted steering/speed (needs `Is_IMU`). |
| `scaled_speed` | `scaled_speed` | Reserved - not currently used by the built-in algorithm. |

## Debug telemetry - `log`

`log <stream> [interval_ms]` toggles that stream on/off; if it's now on and
you gave an interval, that becomes how often it's sent. Streams:

- `dist` - always available: current front/left/right readings.
- `gyro`, `accel`, `yaw`, `pitch` - only listed/available if `Is_IMU` is on.

Example: `log dist 200` starts printing distance readings every 200ms;
sending `log dist` again turns it back off.

## Manual maneuver

| Command | Allowed in | Does |
| --- | --- | --- |
| `180` | Running | Triggers the same turning maneuver as the automatic dead-end turn, but longer (`MANEUVER_180_MS`) and without backing up first - handy for spinning the robot around manually on the bench. |

## Saving

| Command | Requires | Does |
| --- | --- | --- |
| `save` | `Memory` on | Writes every tunable value above to flash. They're loaded back automatically on every future boot - see `Memory` in `config.h`. |
