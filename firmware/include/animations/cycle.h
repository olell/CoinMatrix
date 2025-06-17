#ifndef _ANIM_CYCLE_H
#define _ANIM_CYCLE_H

#include "animation.h"  // for animation_t
#include "charlie.h"    // for matrix function
#include "gfx.h"
#include "systick.h"  // for millis()

// you can declare variables for your animation here
static uint8_t cycle_v, cycle_d;

static void cycle_init(void) {
    // setup your animation
    cycle_v = 0;
    cycle_d = 0;
}

static void cycle_tick() {
    // draw something to matrix

    if (cycle_d)
        cycle_v++;
    else
        cycle_v--;

    if (cycle_v == 0) cycle_d = 1 - cycle_d;

    for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
            gfxSetPixelMappedRGB(x, y, fromHSV((cycle_v + (4 * (x * y))) % 255, 255, 255));
        }
    }
}

static const animation_t cycle_animation = {
    .init = cycle_init,
    .tick = cycle_tick,
    .tick_interval = 15,
};

#endif