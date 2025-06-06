#ifndef _GFX_H
#define _GFX_H

#include <stdint.h>

#include "charlie.h"

void gfxSetPixelXY(uint8_t x, uint8_t y, uint8_t v) {
    charlieSetPixelRaw(y * CHARLIE_PIN_COUNT + ((x >= y) ? x + 1 : x), v);
}

uint8_t gfxGetPixelXY(uint8_t x, uint8_t y) {
    return charlieGetPixelRaw(y * CHARLIE_PIN_COUNT + ((x >= y) ? x + 1 : x));
}

void gfxSetPixelRGB(uint8_t x, uint8_t y, uint32_t color) {
    gfxSetPixelXY((x * 3), y, ((color >> 16) & 0xff));
    gfxSetPixelXY((x * 3) + 1, y, ((color >> 8) & 0xff));
    gfxSetPixelXY((x * 3) + 2, y, (color & 0xff));
}

uint32_t gfxGetPixelRGB(uint8_t x, uint8_t y) {
    const uint8_t r = gfxGetPixelXY((x * 3), y);
    const uint8_t g = gfxGetPixelXY((x * 3) + 1, y);
    const uint8_t b = gfxGetPixelXY((x * 3) + 2, y);
    return ((uint8_t)r << 16) | ((uint8_t)g << 8) | (uint8_t)b;
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

uint32_t gfxGetPixelMappedRGB(uint8_t x, uint8_t y) {
    // corner leds don't exist
    if ((x == 0 || x == 7) && (y == 0 || y == 7)) return 0;

    // right matrix side is actually below left side
    uint8_t rw = y;
    uint8_t cl = x;
    if (cl > 3) {
        cl -= 4;
        rw += 8;
    }

    // check out of bounds
    if (cl > 12 || rw > 15) return 0;

    return gfxGetPixelRGB(cl, rw);
}

void gfxClear() {
    for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
            gfxSetPixelMappedRGB(x, y, 0);
        }
    }
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

/**
 * @brief returns a 32-bit color value from HSV values
 * @param h 0-255
 * @param s 0-255
 * @param v 0-255
 *
 * based on https://github.com/judge2005/arduinoHSV/
 */
uint32_t fromHSV(uint8_t h, uint8_t s, uint8_t v) {
    // this is the algorithm to convert from RGB to HSV
    h = (h * 192) / 256;       // 0..191
    uint8_t i = h / 32;        // We want a value of 0 thru 5
    uint8_t f = (h % 32) * 8;  // 'fractional' part of 'i' 0..248 in jumps

    uint8_t sInv = 255 - s;       // 0 -> 0xff, 0xff -> 0
    uint8_t fInv = 255 - f;       // 0 -> 0xff, 0xff -> 0
    uint8_t pv = v * sInv / 256;  // pv will be in range 0 - 255
    uint8_t qv = v * (256 - s * f / 256) / 256;
    uint8_t tv = v * (256 - s * fInv / 256) / 256;

    switch (i) {
        case 0:
            return fromRGB8(v, tv, pv);
        case 1:
            return fromRGB8(qv, v, pv);
        case 2:
            return fromRGB8(pv, v, tv);
        case 3:
            return fromRGB8(pv, qv, v);
        case 4:
            return fromRGB8(tv, pv, v);
        case 5:
            return fromRGB8(v, pv, qv);
        default:
            return 0;
    }
}

#endif  // _GFX_H