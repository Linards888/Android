# Calibration

## TOF (VL53L0X)

Nothing to calibrate by hand. Each sensor is assigned its I2C address (from
`TOF_SENSOR_LIST` in `config.h`) automatically at boot, one at a time over
its XSHUT pin. If a sensor fails to initialize you'll see `TOF init failed:
<name>` on the Serial Monitor - double check its XSHUT pin and wiring.

## Sharp IR distance sensors

Sharp sensors output a voltage that's a curve (not a straight line) against
distance, and that curve differs by exact part number. Rather than guess a
formula for your specific sensor, this firmware uses a simple two-point
straight-line calibration, good enough over the ~10-80cm range a folkrace
track needs:

1. Upload the firmware with `Is_Sharp` enabled and `Is_blueTooth` enabled
   (or just watch Serial output with a temporary debug print).
2. Place a flat wall/object at a known **near** distance (e.g. 100mm) in
   front of the sensor. Read the raw ADC value - easiest way is to
   temporarily add `Serial.println(analogRead(<pin>));` in `setup()`, or
   connect over BLE and use `log dist 200` once wired into `sensor_read()`.
3. Enter that raw value as `SHARP_ADC_AT_NEAR` and the distance as
   `SHARP_NEAR_MM` in `config.h`.
4. Repeat at a known **far** distance (e.g. 800mm), filling in
   `SHARP_ADC_AT_FAR` / `SHARP_FAR_MM`.
5. Re-upload. Readings between (and a bit beyond) those two points will now
   read out in millimeters; `sharp_read()` clamps to the near/far range so a
   noisy reading can't report an impossible distance.

Repeat separately for each Sharp sensor if they don't behave identically
(different units of the same part usually only need one calibration for all
of them).

## Ultrasonic (HC-SR04 style)

No calibration needed - distance is computed directly from the echo pulse
timing using the speed of sound. If readings look off, check `USONIC_TIMEOUT_US`
in `config.h` (too short cuts off real echoes from farther objects) and make
sure the sensor's trig/echo pins aren't swapped.

## IMU

Run the BLE `calibrate` command (or enter `Mode::CALIBRATION` - it's what the
"calibrate" command does) with the robot completely flat and still. This
measures the accelerometer/gyro bias and removes it from every reading after
that. Recalibrate if you change the IMU's mounting orientation or notice the
`pitch`/`gyro` debug log drifting while the robot is stationary.
