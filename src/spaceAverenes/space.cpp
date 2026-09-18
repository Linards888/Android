#include "space.h"

#if FEATURE_SPACE_AWARENESS

static RobotPose pose;

void space_awareness_init() {
    // TODO: reset pose, allocate/clear the map once one exists.
    pose = RobotPose();
}

void space_awareness_update(float dt_seconds) {
    (void)dt_seconds;
    // TODO: integrate pose from IMU yaw + speed, paint sensor readings
    // into a map, and eventually feed a planned path back out.
}

RobotPose space_awareness_get_pose() {
    return pose;
}

#endif
