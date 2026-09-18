# Documentation

New to this repo? Read these in order:

1. **[Necessities](Necessities.md)** - the hardware, tools and software you need before you touch any code.
2. **[Setup](Setup.md)** - install, configure `config.h`, upload, and bring the robot up on the bench.

Once it's running:

- **[Algorithm](Algorithm.md)** - a complete worked example (PID wall-following) for the `RUNNING` state, step by step: how it decides where to steer and how fast to go. `onRunning()` ships blank - this is a starting point, not what's currently running.
- **[Editing Android.ino](Android_ino.md)** - what's already in the file, what's safe to change, and the full toolbox of functions it can call into.
- **[Architecture](Arhitecture.md)** - how the code is organized, the full state machine, and how to extend it (new sensor type, new command, different driving algorithm).
- **[BLE Commands](BLE_Commands.md)** - the full BLE tuning console reference.
- **[Calibration](Calibration.md)** - calibrating Sharp IR sensors and the IMU.

`config.h` and `Android.ino` are the two files you edit to run this on your
own robot - `Defaults.h` holds starting values you can tweak later, and
everything under `src/` is the driver/plumbing layer the docs above
explain.
