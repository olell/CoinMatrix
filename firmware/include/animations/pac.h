#ifndef _ANIM_PACMAN_CHOMP_H
#define _ANIM_PACMAN_CHOMP_H

#include "animation.h"
#include "charlie.h"
#include "gfx.h"
#include "systick.h"

#define PACMAN_COLOR fromRGB8(255, 255, 0)
#define BG_COLOR fromRGB8(0, 0, 0)

static uint8_t frame = 0;
static int8_t pacman_x = 0;

static uint8_t direction = 0;

const uint8_t pacman_frames[3][8] = {
    {0b00111100,
     0b01111110,
     0b11111111,
     0b11111111,
     0b11111111,
     0b11111111,
     0b01111110,
     0b00111100},
    {0b00111100,
     0b01111110,
     0b11111000,
     0b11100000,
     0b11111000,
     0b11111111,
     0b01111110,
     0b00111100},
    {0b00111100,
     0b01111110,
     0b11111000,
     0b11100000,
     0b11100000,
     0b11111000,
     0b01111110,
     0b00111100}};

static void draw_pacman(int8_t x_offset, uint8_t frame_id, uint8_t direction) {
    for (uint8_t y = 0; y < 8; y++) {
        uint8_t row = pacman_frames[frame_id][y];
        for (uint8_t x = 0; x < 8; x++) {
            if ((row >> (7 - x)) & 0x01) {
                int px = direction ? 8 - (x + x_offset) : x + x_offset;
                if (px >= 0 && px < 8) {
                    gfxSetPixelMappedRGB(px, y, PACMAN_COLOR);
                }
            }
        }
    }
}

static void pac_init(void) {
    frame = 0;
    pacman_x = -7;
    direction = 0;
}

static void pac_tick() {
    gfxClear();
    draw_pacman(pacman_x, frame % 3, direction);

    frame++;
    if (frame % 2 == 0) {
        pacman_x++;
    }

    if (pacman_x > 8) {
        pacman_x = -7;
        direction = 1 - direction;
    }
}

static const animation_t pac_animation = {
    .init = pac_init,
    .tick = pac_tick,
    .tick_interval = 100,
};

#endif
