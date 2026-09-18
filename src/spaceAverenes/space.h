#pragma once
#include <Arduino.h>
#include "config.h"

// ============================================================
//  Space Awareness — NOT IMPLEMENTED YET.
// ============================================================
//  This is a placeholder for a future feature: using IMU (and
//  maybe wheel-odometry / sensor sweep) readings to build up an
//  estimate of the robot's position and the track's shape, then
//  use that map to plan a better path than "follow the wall in
//  front of me right now".
//
//  Nothing in this file runs today — FEATURE_SPACE_AWARENESS
//  gates it off completely, and space_awareness_init()/update()
//  are empty stubs. It exists so that:
//    - there's an obvious, conventional place to build this later
//    - the rest of the codebase (Android.ino, config.h) already
//      has the on/off switch and the call sites wired up
//
//  A rough sketch of what this will probably need, when it's
//  built for real:
//    - A pose estimate (x, y, heading) updated every tick from
//      IMU yaw (see src/IMU) plus some notion of speed/time or
//      wheel odometry, since Folkrace doesn't have wheel encoders
//      today.
//    - A map representation for the track — likely an occupancy
//      grid (mm-scale cells, "wall" / "free" / "unknown") that
//      gets painted in using the side/front distance sensors
//      (src/Sensors/DistanceSensors.h) as the robot drives.
//    - A path-planning step that turns the map + current pose
//      into a suggested speed/steer, to blend with (or replace)
//      the reactive PID logic in Android.ino's RUNNING case.
//    - Pose drift correction — pure IMU-yaw + speed-based
//      dead-reckoning drifts fast; this will likely need
//      loop-closure or landmark matching against the map to stay
//      usable for a whole lap.
//
//  None of that is implemented here. Treat this file as scaffolding,
//  not a working feature.
// ============================================================

#if FEATURE_SPACE_AWARENESS

struct RobotPose {
    float x_mm = 0;
    float y_mm = 0;
    float heading_deg = 0;
};

void space_awareness_init();
void space_awareness_update(float dt_seconds);
RobotPose space_awareness_get_pose();

#endif
