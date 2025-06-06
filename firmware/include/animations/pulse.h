#ifndef _ANIM_PULSE_H
#define _ANIM_PULSE_H

#include "animation.h"  // for animation_t
#include "charlie.h"    // for matrix function
#include "systick.h"    // for millis()

// you can declare variables for your animation here
static uint8_t pulse_v, pulse_d;

static void pulse_init(void) {
    // setup your animation
    pulse_v = 0;
    pulse_d = 1;
}

static void pulse_tick() {
    // draw something to matrix

    if (pulse_d) pulse_v++;
    if (!pulse_d) pulse_v--;
    if (pulse_v == CHARLIE_MAX_BRIGHTNESS) pulse_d = 0;
    if (pulse_v == 0) pulse_d = 1;

    for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
            charlieSetPixelMappedRGB(x, y, pulse_v, pulse_v, pulse_v);
        }
    }
}

static const animation_t pulse_animation = {
    .init = pulse_init,
    .tick = pulse_tick,
    .tick_interval = 50,
};

#endif