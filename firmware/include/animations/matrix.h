#ifndef _ANIM_MATRIX_H
#define _ANIM_MATRIX_H

#include "animation.h"
#include "ch32fun.h"
#include "charlie.h"
#include "gfx.h"
#include "lib_rand.h"
#include "systick.h"

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define MAX_STREAMS 8

typedef struct {
    int8_t y;
    uint8_t length;
    uint8_t speed;  // in ticks
    uint8_t counter;
} rain_stream_t;

static rain_stream_t streams[MAX_STREAMS];

static void matrix_init(void) {
    for (int i = 0; i < MAX_STREAMS; ++i) {
        streams[i].y = -(rand() % MATRIX_HEIGHT);  // start offscreen
        streams[i].length = 3 + rand() % 3;        // 3 to 5 pixels long
        streams[i].speed = 2 + rand() % 4;         // lower is faster
        streams[i].counter = 0;
    }
}

static void matrix_tick() {
    gfxClear();  // clear screen each frame

    for (int x = 0; x < MAX_STREAMS; ++x) {
        rain_stream_t *stream = &streams[x];

        stream->counter++;
        if (stream->counter >= stream->speed) {
            stream->counter = 0;
            stream->y++;

            // reset stream if it goes offscreen
            if (stream->y - stream->length > MATRIX_HEIGHT) {
                stream->y = -1 * (rand() % MATRIX_HEIGHT);
                stream->length = 3 + rand() % 3;
                stream->speed = 2 + rand() % 4;
            }
        }

        for (int i = 0; i < stream->length; ++i) {
            int y_pos = stream->y - i;
            if (y_pos >= 0 && y_pos < MATRIX_HEIGHT) {
                uint8_t brightness = (i == 0) ? 255 : (200 - i * 50);  // brighter head
                uint32_t color = fromRGB8(0, brightness, 0);
                gfxSetPixelMappedRGB(x, y_pos, color);
            }
        }
    }
}

static const animation_t matrix_animation = {
    .init = matrix_init,
    .tick = matrix_tick,
    .tick_interval = 50,  // control overall speed
};

#endif
