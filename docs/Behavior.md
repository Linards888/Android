# Behavior

This content moved back to [Android_ino.md](Android_ino.md).

The state machine, `setup()`, `loop()`, and the driving algorithm
(`onRunning()`) now live directly in `Android.ino` again - there is no
separate `src/Behavior/Behavior.cpp` module anymore. See
[Android_ino.md](Android_ino.md) for what's in the file, what's safe to
change, and the full toolbox of functions you can call from it.
