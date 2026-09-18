# Calibration

The `calibrate` command (BLE or Serial) puts the robot into
`RunningState::CALIBRATING` for a few seconds. This is **a live sensor
dump, not automatic tuning** — it exists so you can watch real numbers
while positioning the robot, and use them to set good values by hand. See
`src/Calibrations/Calibration.cpp` for the implementation.

## Running it

1. Connect over BLE (or Serial at 115200 baud).
2. Send `calibrate`.
3. For ~3 seconds, every enabled distance sensor's current reading prints
   roughly 5x/second: `left: 812`, `front: 143 mm`, etc.
4. If `FEATURE_IMU` is on and `imu_enabled` is set, the IMU's gyro/accel
   bias calibration also runs at the start — **hold the robot still and
   level** while that happens (you'll see `calibrating imu, keep the robot
   still...` then `imu calibrated`).
5. It returns to `READY` automatically when done.

## Using the numbers

**Front sensor (`dist_near` / `dist_far` / `esc_trig_mm`):** hold the robot
at the closest distance from a wall you want it to still be driving at full
speed, note the front reading, `set dist_near <that value>`. Repeat at the
farthest distance you want it to bother slowing down for, `set dist_far
<that value>`. Hold it at the distance where it should treat "wall" as
"about to hit it" and back off, `set esc_trig_mm <that value>`.

Remember: for a Sharp IR sensor, this is a smoothed **raw ADC count**, not a
real distance (see `src/Sensors/sharp_logic.cpp`) — smaller isn't
necessarily "closer" unless you've checked your specific sensor's response
curve. For a VL53L0X or ultrasonic sensor it's real millimeters, so smaller
really does mean closer.

**Steering weights (`w_<sensor name>`):** with the robot equidistant from
both walls, watch `left`/`right` (or whatever your sensors are named) — they
should read close to equal. If the robot pulls toward one side while
driving straight, that side's sensor is either miscalibrated or its weight's
sign/magnitude needs adjusting (`set w_left <value>`, `set w_right <value>`
— see the `role`/`weight` explanation in `configs/config_template.h` and
`docs/Arhitecture.md`).

**PID gains (`kp`/`ki`/`kd`):** not something `calibrate` measures for you —
tune these the usual way: raise `kp` until it oscillates around the line,
back off a bit, add a small amount of `kd` to damp the oscillation, only add
`ki` if there's a persistent steady-state offset.

Once you're happy, `save` persists every tuned value to flash (requires
`FEATURE_MEMORY` on) so it survives a power cycle.
