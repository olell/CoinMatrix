#ifndef _ANIM_ROLLING_TEXT_H
#define _ANIM_ROLLING_TEXT_H

#include "animation.h"  // for animation_t
#include "font.h"
#include "gfx.h"  // for matrix function
#include "stdio.h"
#include "string.h"
#include "systick.h"  // for millis()

// you can declare variables for your animation here
static const uint8_t rolling_text_strings_count = 1;
static const char* rolling_text_strings[] = {
    "HALLO WELT   ",
};
static uint8_t rolling_text_string_index = 0;
static const char* rolling_text_display_string;
static uint8_t rolling_text_display_string_length;

static uint8_t rolling_text_current_letter;
static uint8_t rolling_text_current_letter_col;
static uint8_t rolling_text_current_color;

static void rolling_text_init(void) {
    // setup your animation
    rolling_text_display_string = rolling_text_strings[0];
    rolling_text_string_index = 0;
    rolling_text_display_string_length = strlen(rolling_text_display_string);

    rolling_text_current_letter = 0;
    rolling_text_current_letter_col = 0;
    rolling_text_current_color = 0;
}

#define COLOR (fromRGB8(255, 0, 0))

static void rolling_text_tick() {
    // draw something to matrix

    // shift out current pixels
    for (uint8_t x = 1; x < MATRIX_WIDTH; x++) {
        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
            gfxSetPixelMappedRGB(x - 1, y, gfxGetPixelMappedRGB(x, y));
            gfxSetPixelMappedRGB(x, y, 0);
        }
    }

    for (uint8_t y = 0; y < 6; y++) {
        uint8_t byte = font[(rolling_text_display_string[rolling_text_current_letter] - 32) * 6 + y];
        uint8_t bit = byte >> (8 - rolling_text_current_letter_col);
        gfxSetPixelMappedRGB(7, y + 1, (bit & 1) ? (fromHSV(rolling_text_current_color, 255, 255)) : 0);
    }

    rolling_text_current_letter_col++;
    rolling_text_current_color += 8;
    if (rolling_text_current_letter_col == 4) {
        rolling_text_current_letter_col = 0;
        rolling_text_current_letter = (rolling_text_current_letter + 1) % rolling_text_display_string_length;
    }
}

static const animation_t rolling_text_animation = {
    .init = rolling_text_init,
    .tick = rolling_text_tick,
    .tick_interval = 150,
};

#endif