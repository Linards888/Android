#include "Params.h"

// ---- PID ----
float kp = 0.0f, ki = 0.0f, kd = 0.0f;
float kleft = 0.0f, kright = 0.0f;
int maxdelta = 0, constrainpid = 0, constraindelta = 0;

// ---- Drive speeds ----
int maxspeed = 0, minspeed = 0, fspeed = 0, rspeed = 0;

// ---- Sensor thresholds ----
int errorleftdist = 0, errorrightdist = 0, minwalldistFront = 0;

// ---- Motor / actuator tuning ----
// Same defaults the original Android.ino had these as.
int DRIVE_MAX = 255;

int PWM_FREQ_HZ = 20000;      // 20kHz - above human hearing
int PWM_RESOLUTION_BITS = 8;  // 0-255 duty steps

int ESC_MIN_US = 1000;      // full reverse (or full-off, on ESCs with no reverse)
int ESC_NEUTRAL_US = 1500;  // stop
int ESC_MAX_US = 2000;      // full forward

int STEER_MIN_DEG = 45;
int STEER_CENTER_DEG = 90;
int STEER_MAX_DEG = 135;

int SERVO_PULSE_MIN_US = 500;   // 0 degrees
int SERVO_PULSE_MAX_US = 2500;  // 180 degrees

// ---- Debug ----
DebugState debugState;
