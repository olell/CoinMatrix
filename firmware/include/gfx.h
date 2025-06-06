#ifndef _GFX_H
#define _GFX_H

#include <stdint.h>

#include "charlie.h"

void gfxSetPixelXY(uint8_t x, uint8_t y, uint8_t v) {
    charlieSetPixelRaw(y * CHARLIE_PIN_COUNT + ((x >= y) ? x + 1 : x), v);
}

void gfxSetPixelRGB(uint8_t x, uint8_t y, uint32_t color) {
    gfxSetPixelXY((x * 3), y, ((color >> 16) & 0xff));
    gfxSetPixelXY((x * 3) + 1, y, ((color >> 8) & 0xff));
    gfxSetPixelXY((x * 3) + 2, y, (color & 0xff));
}

void gfxSetPixelMappedRGB(uint8_t x, uint8_t y, uint32_t color) {
    // corner leds don't exist
    if ((x == 0 || x == 7) && (y == 0 || y == 7)) return;

    // right matrix side is actually below left side
    uint8_t rw = y;
    uint8_t cl = x;
    if (cl > 3) {
        cl -= 4;
        rw += 8;
    }

    // check out of bounds
    if (cl > 12 || rw > 15) return;

    gfxSetPixelRGB(cl, rw, color);
}

/**
 * @brief returns a 32-bit color value from RGB values which
 *        get reduced to the pwm levels the matrix is able to do
 */
uint32_t fromRGB8(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t rm = (r >> (8 - CHARLIE_PWM_BITS));
    uint32_t gm = (g >> (8 - CHARLIE_PWM_BITS));
    uint32_t bm = (b >> (8 - CHARLIE_PWM_BITS));
    return ((rm << 16) | (gm << 8) | bm);
}

/**
 * @brief returns a 32-bit color value from RGB values which aren't modified
 */
uint32_t fromRGB(uint8_t r, uint8_t g, uint8_t b) {
    return (((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b);
}

#endif  // _GFX_H