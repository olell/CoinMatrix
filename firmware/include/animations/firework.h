#ifndef _ANIM_FIREWORK_H
#define _ANIM_FIREWORK_H

#include <stdint.h>

#include "animation.h"
#include "ch32fun.h"
#include "charlie.h"
#include "gfx.h"
#include "lib_rand.h"
#include "systick.h"

#define MAX_PARTICLES 12
#define FIXED_SHIFT 4  // 1.4 fixed-point
#define FIXED_ONE (1 << FIXED_SHIFT)
#define FIXED_HALF (1 << (FIXED_SHIFT - 1))
#define GRAVITY 1  // fixed-point value, affects vertical speed
#define FADE_RATE 10

typedef struct {
    int16_t x, y;   // 4.4 fixed-point position
    int8_t dx, dy;  // 4.4 fixed-point velocity
    uint8_t brightness;
    uint8_t hue;
    uint8_t active;
} particle_t;

static enum { FIREWORK_ASCEND,
              FIREWORK_EXPLODE } state;
static int16_t firework_y;  // fixed-point vertical position
static uint8_t explosion_y;
static uint8_t explosion_hue;
static particle_t particles[MAX_PARTICLES];

static void launch_firework() {
    firework_y = 7 << FIXED_SHIFT;
    explosion_y = (rand() % 4 + 2);  // between 2 and 5
    explosion_hue = rand() % 256;
    state = FIREWORK_ASCEND;
}

static void start_explosion() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        // Predefined angles as dx/dy pairs (8-bit fixed approximations)
        // These simulate a radial burst pattern
        int8_t angle_table[12][2] = {
            {8, 0}, {6, 4}, {4, 6}, {0, 8}, {-4, 6}, {-6, 4}, {-8, 0}, {-6, -4}, {-4, -6}, {0, -8}, {4, -6}, {6, -4}};

        particles[i].x = 4 * FIXED_ONE;  // center X
        particles[i].y = firework_y;
        particles[i].dx = angle_table[i][0];
        particles[i].dy = angle_table[i][1] / 2;  // smaller vertical boost
        particles[i].brightness = 255;
        particles[i].hue = explosion_hue + (i * 8);
        particles[i].active = 1;
    }
    state = FIREWORK_EXPLODE;
}

static void firework_init(void) {
    launch_firework();
}

static void firework_tick() {
    gfxClear();

    if (state == FIREWORK_ASCEND) {
        firework_y -= FIXED_ONE / 2;  // Move upward

        uint8_t fy = (firework_y + FIXED_HALF) >> FIXED_SHIFT;
        if (fy < 8) {
            gfxSetPixelMappedRGB(4, fy, fromRGB8(255, 255, 255));
        }

        if (fy <= explosion_y) {
            start_explosion();
        }

    } else if (state == FIREWORK_EXPLODE) {
        uint8_t all_inactive = 1;

        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) continue;

            // Update position
            particles[i].x += particles[i].dx;
            particles[i].y += particles[i].dy;
            particles[i].dy += GRAVITY;

            // Fade brightness
            if (particles[i].brightness > FADE_RATE) {
                particles[i].brightness -= FADE_RATE;
            } else {
                particles[i].active = 0;
                continue;
            }

            // Draw particle
            int8_t px = (particles[i].x + FIXED_HALF) >> FIXED_SHIFT;
            int8_t py = (particles[i].y + FIXED_HALF) >> FIXED_SHIFT;
            if (px >= 0 && px < 8 && py >= 0 && py < 8) {
                uint32_t color = fromHSV(particles[i].hue, 255, particles[i].brightness);
                gfxSetPixelMappedRGB(px, py, color);
            }

            all_inactive = 0;
        }

        if (all_inactive) {
            launch_firework();
        }
    }
}

static const animation_t firework_animation = {
    .init = firework_init,
    .tick = firework_tick,
    .tick_interval = 50,  // ms
};

#endif
