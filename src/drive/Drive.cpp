#include "Drive.h"
#include "../Utils/RobotState.h"
#include "../Utils/Notify.h"

#if (MOTOR_KIND == MOTOR_BRUSHLESS) || (STEERING_MODE == STEERING_SERVO)
    #include <ESP32Servo.h>
#endif

// ---- generate one Motor instance + a pointer table from MOTOR_LIST ----

#define X(name, side, pinA, pinB) Motor motor_##name = { #name, side, pinA, pinB };
    MOTOR_LIST
#undef X

#define X(name, side, pinA, pinB) &motor_##name,
    Motor* all_motors[] = { MOTOR_LIST };
#undef X

const uint8_t MOTOR_COUNT = sizeof(all_motors) / sizeof(all_motors[0]);

// ---- brushless ESC channels (only allocated if actually needed) ----

#if MOTOR_KIND == MOTOR_BRUSHLESS
    static Servo esc[DRIVE_MAX_MOTORS];
#endif

// ---- steering servo (only allocated if actually needed) ----

#if STEERING_MODE == STEERING_SERVO
    static Servo steering_servo;

    #ifndef STEERING_SERVO_CENTER_US
        #define STEERING_SERVO_CENTER_US 1500
    #endif
    #ifndef STEERING_SERVO_RANGE_US
        #define STEERING_SERVO_RANGE_US 500  // +/- this many us from center at full steer lock
    #endif
#endif

// ---- writing a single motor's speed ----

static void motor_write(uint8_t index, int16_t speed) {
    Motor* m = all_motors[index];
    speed = constrain(speed, (int16_t)-255, (int16_t)255);
    if (state.drive_reversed) speed = -speed;

#if MOTOR_KIND == MOTOR_BRUSHED
    if (speed >= 0) {
        analogWrite(m->pinA, speed);
        analogWrite(m->pinB, 0);
    } else {
        analogWrite(m->pinA, 0);
        analogWrite(m->pinB, -speed);
    }
#elif MOTOR_KIND == MOTOR_BRUSHLESS
    int us = 1500 + (int)((long)speed * 500L / 255L);
    esc[index].writeMicroseconds(us);
#endif
}

void drive_init() {
    if (MOTOR_COUNT > DRIVE_MAX_MOTORS) {
        notify("ERROR: %d motors configured but Drive.cpp only supports %d. Raise DRIVE_MAX_MOTORS.\n",
               MOTOR_COUNT, DRIVE_MAX_MOTORS);
    }

#if MOTOR_KIND == MOTOR_BRUSHED
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
        pinMode(all_motors[i]->pinA, OUTPUT);
        pinMode(all_motors[i]->pinB, OUTPUT);
    }
#elif MOTOR_KIND == MOTOR_BRUSHLESS
    for (uint8_t i = 0; i < MOTOR_COUNT && i < DRIVE_MAX_MOTORS; i++) {
        esc[i].setPeriodHertz(50);
        esc[i].attach(all_motors[i]->pinA, 1000, 2000);
    }
#endif

#if STEERING_MODE == STEERING_SERVO
    steering_servo.setPeriodHertz(50);
    steering_servo.attach(STEERING_SERVO_PIN, 1000, 2000);
#endif

    drive_stop();
}

void drive_set(int16_t throttle, int16_t steer) {
    throttle = constrain(throttle, (int16_t)-255, (int16_t)255);
    steer = constrain(steer, (int16_t)-255, (int16_t)255);

#if STEERING_MODE == STEERING_SERVO
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) motor_write(i, throttle);

    int us = STEERING_SERVO_CENTER_US + (int)((long)steer * STEERING_SERVO_RANGE_US / 255L);
    steering_servo.writeMicroseconds(us);

#elif STEERING_MODE == STEERING_DIFFERENTIAL
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) {
        Motor* m = all_motors[i];
        int16_t speed = throttle;
        if (m->side == SIDE_LEFT)       speed = throttle - steer;
        else if (m->side == SIDE_RIGHT) speed = throttle + steer;
        motor_write(i, speed);
    }

#else // STEERING_NONE
    for (uint8_t i = 0; i < MOTOR_COUNT; i++) motor_write(i, throttle);
#endif
}

void drive_stop() {
    drive_set(0, 0);
}

void drive_do_180() {
#if STEERING_MODE == STEERING_DIFFERENTIAL
    drive_set(0, 200);   // equal & opposite speeds -> spin in place
    delay(400);
    drive_stop();
#elif STEERING_MODE == STEERING_SERVO
    drive_set(-180, 255); // reverse with full steering lock
    delay(400);
    drive_stop();
#else
    notify("180 not supported: this chassis has STEERING_NONE (can't turn)\n");
#endif
}

/* ---- Usage, from Android.ino ----

  drive_init();                 // once, in setup()

  drive_set(180, 0);            // drive straight ahead
  drive_set(150, -80);          // drive forward, steering left
  drive_stop();                 // motors off
  drive_do_180();                // debug: spin/turn around

*/
