#include "Memory.h"

#if Memory
#include <Preferences.h>
#include "RobotState.h"

static const char* NAMESPACE = "folkrace";

void load_state() {
  Preferences prefs;
  prefs.begin(NAMESPACE, /*readOnly=*/true);

  state.pid.kp             = prefs.getFloat("kp",     state.pid.kp);
  state.pid.ki             = prefs.getFloat("ki",     state.pid.ki);
  state.pid.kd             = prefs.getFloat("kd",     state.pid.kd);
  state.pid.k_left         = prefs.getFloat("kleft",  state.pid.k_left);
  state.pid.k_left_side    = prefs.getFloat("klside", state.pid.k_left_side);
  state.pid.k_right        = prefs.getFloat("kright", state.pid.k_right);
  state.pid.k_right_side   = prefs.getFloat("krside", state.pid.k_right_side);
  state.k_reverse          = prefs.getFloat("krev",   state.k_reverse);

  state.speed_forward      = prefs.getInt("spdfwd",   state.speed_forward);
  state.speed_reverse      = prefs.getInt("spdrev",   state.speed_reverse);
  state.speed_min          = prefs.getInt("spdmin",   state.speed_min);
  state.speed_max          = prefs.getInt("spdmax",   state.speed_max);
  state.scaled_speed       = prefs.getBool("scaled",  state.scaled_speed);

  state.dist_near          = prefs.getInt("dnear",    state.dist_near);
  state.dist_far           = prefs.getInt("dfar",     state.dist_far);
  state.dist_reverse       = prefs.getInt("drev",     state.dist_reverse);
  state.dist_constrain     = prefs.getInt("dconstr",  state.dist_constrain);

  state.drive_reversed     = prefs.getBool("drvrev",  state.drive_reversed);
  state.accel              = prefs.getInt("accel",    state.accel);
  state.brake              = prefs.getInt("brake",    state.brake);

  state.imu_enabled        = prefs.getBool("imuen",   state.imu_enabled);
  state.k_pitch_running    = prefs.getFloat("kpitch", state.k_pitch_running);
  state.k_accel_nudge      = prefs.getFloat("kanudge",state.k_accel_nudge);
  state.slope_threshold    = prefs.getFloat("slopeT", state.slope_threshold);
  state.slope_boost        = prefs.getBool("slopeB",  state.slope_boost);

  state.start_delay_ms     = prefs.getULong("startdl",state.start_delay_ms);

  prefs.end();
}

void save_state() {
  Preferences prefs;
  prefs.begin(NAMESPACE, /*readOnly=*/false);

  prefs.putFloat("kp",      state.pid.kp);
  prefs.putFloat("ki",      state.pid.ki);
  prefs.putFloat("kd",      state.pid.kd);
  prefs.putFloat("kleft",   state.pid.k_left);
  prefs.putFloat("klside",  state.pid.k_left_side);
  prefs.putFloat("kright",  state.pid.k_right);
  prefs.putFloat("krside",  state.pid.k_right_side);
  prefs.putFloat("krev",    state.k_reverse);

  prefs.putInt("spdfwd",    state.speed_forward);
  prefs.putInt("spdrev",    state.speed_reverse);
  prefs.putInt("spdmin",    state.speed_min);
  prefs.putInt("spdmax",    state.speed_max);
  prefs.putBool("scaled",   state.scaled_speed);

  prefs.putInt("dnear",     state.dist_near);
  prefs.putInt("dfar",      state.dist_far);
  prefs.putInt("drev",      state.dist_reverse);
  prefs.putInt("dconstr",   state.dist_constrain);

  prefs.putBool("drvrev",   state.drive_reversed);
  prefs.putInt("accel",     state.accel);
  prefs.putInt("brake",     state.brake);

  prefs.putBool("imuen",    state.imu_enabled);
  prefs.putFloat("kpitch",  state.k_pitch_running);
  prefs.putFloat("kanudge", state.k_accel_nudge);
  prefs.putFloat("slopeT",  state.slope_threshold);
  prefs.putBool("slopeB",   state.slope_boost);

  prefs.putULong("startdl", state.start_delay_ms);

  prefs.end();
}

#endif
