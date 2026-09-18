#pragma once
#include <Arduino.h>
#include "config.h"
#include "../Utils/HardwareEnums.h"

// ============================================================
//  Drive — the motor abstraction.
// ============================================================
//  Android.ino should never touch a pin directly. It calls:
//
//      drive_set(throttle, steer);   // throttle & steer: -255..255
//      drive_stop();
//      drive_do_180();
//
//  and Drive.cpp figures out what that means for YOUR chassis —
//  1/2/4 motors, brushed or brushless, differential or servo
//  steering — based on the DRIVE_TOPOLOGY / STEERING_MODE /
//  MOTOR_KIND / MOTOR_LIST choices in your configs/config_*.h.
// ============================================================

#if !defined(DRIVE_TOPOLOGY) || !defined(STEERING_MODE) || !defined(MOTOR_KIND) || !defined(MOTOR_LIST)
    #error "Your config is missing DRIVE_TOPOLOGY / STEERING_MODE / MOTOR_KIND / MOTOR_LIST. See configs/config_template.h."
#endif

#if DRIVE_TOPOLOGY == DRIVE_1_MOTOR && STEERING_MODE == STEERING_DIFFERENTIAL
    #error "A single motor can't do differential steering. Use STEERING_SERVO or STEERING_NONE."
#endif

#if STEERING_MODE == STEERING_SERVO && !defined(STEERING_SERVO_PIN)
    #error "STEERING_MODE is STEERING_SERVO but STEERING_SERVO_PIN isn't defined in your config."
#endif

#define DRIVE_MAX_MOTORS 8

struct Motor {
    const char* name;
    uint8_t side;   // SIDE_LEFT / SIDE_RIGHT / SIDE_NONE
    uint8_t pinA;   // brushed: forward-ish pin   | brushless: ESC signal pin
    uint8_t pinB;   // brushed: reverse-ish pin   | brushless: PIN_NONE (unused)
};

#define X(name, side, pinA, pinB) extern Motor motor_##name;
    MOTOR_LIST
#undef X

extern Motor* all_motors[];
extern const uint8_t MOTOR_COUNT;

void drive_init();

// throttle: forward(+)/backward(-) speed, -255..255
// steer:    turn amount, -255 (full left) .. +255 (full right), 0 = straight
void drive_set(int16_t throttle, int16_t steer);

void drive_stop();

// A quick spin/turn-around maneuver, used by the debug `180` command.
// Blocking for a couple hundred ms (matches the original firmware's
// behaviour) — only ever called from a manual debug command, not from
// the control loop itself.
void drive_do_180();
