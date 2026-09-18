# Documentation

New to this repo? Read these in order:

1. **[Necessities](Necessities.md)** - the hardware, tools and software you need before you touch any code.
2. **[Setup](Setup.md)** - install, configure `config.h`, upload, and bring the robot up on the bench.

Once it's running:

- **[Algorithm](Algorithm.md)** - exactly what happens in the `RUNNING` state, step by step: how the robot decides where to steer and how fast to go.
- **[Architecture](Arhitecture.md)** - how the code is organized, the full state machine, and how to extend it (new sensor type, new command, different driving algorithm).
- **[BLE Commands](BLE_Commands.md)** - the full BLE tuning console reference.
- **[Calibration](Calibration.md)** - calibrating Sharp IR sensors and the IMU.

`config.h` and `Android.ino` are the only two files you should need to edit
to run this on your own robot - everything under `src/` is the
driver/plumbing layer the docs above explain.
