/**
 * animation player.
 *
 * Format:
 * an animation is a list of instructions. Each instruction is 3 bytes long.
 * The first byte of the instruction is the control byte, the second and third
 * byte contain color information.
 * Each instruction describes a change of a pixel.
 *
 *
 * Control byte:
 * 7 6 5 4 3 2 1 0
 * F F Y Y Y X X X
 * where X, Y are position and F is a flag byte
 * F = 0 -> continue
 * F = 1 -> -- reserved
 * F = 2 -> -- reserved
 * F = 3 -> frame end
 *
 * First Color Byte:
 * 7 6 5 4 3 2 1 0
 * - - - - R R R R
 * where - is reserved, R is red color
 *
 * Second Color Byte:
 * 7 6 5 4 3 2 1 0
 * G G G G B B B B
 * where G is green and B is blue color
 */

#ifndef _ANIMATION_PLAYER_H
#define _ANIMATION_PLAYER_H

#include "animation.h"
#include "gfx.h"
#include "gif.h"

static uint32_t player_position;
static uint8_t anim_invalid = 0;

static void animation_player_init(void) {
    player_position = 0;

    // animation must be dividable by 3
    if (GIF_LENGTH % 3 != 0) {
        anim_invalid = 1;
        return;
    }
}

static void animation_player_tick(void) {
    if (anim_invalid) return;

    if (player_position == 0)
        gfxClear();

    uint8_t ctrl = 0;
    uint8_t r, g, b;
    do {
        ctrl = GIF_INSTRUCTIONS[player_position];
        r = (GIF_INSTRUCTIONS[player_position + 1] & 0xf);
        g = (GIF_INSTRUCTIONS[player_position + 2] & 0xf0) >> 4;
        b = (GIF_INSTRUCTIONS[player_position + 2] & 0xf);

        gfxSetPixelMappedRGB(ctrl & 0x7, (ctrl >> 3) & 0x7, fromRGB(r, g, b));

        player_position += 3;
        if (player_position == GIF_LENGTH) {
            player_position = 0;
        }
    } while ((ctrl & 0xc0) != 0xc0);  // check for next frame flag
}

static animation_t animation_player = {
    .init = animation_player_init,
    .tick = animation_player_tick,
    .tick_interval = 100,
};

#endif  // _ANIMATION_PLAYER_H