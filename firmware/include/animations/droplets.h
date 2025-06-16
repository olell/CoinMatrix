#ifndef _ANIM_DROPLETS_H
#define _ANIM_DROPLETS_H

#include "animation.h"  // for animation_t
#include "charlie.h"    // for matrix function
#include "gfx.h"
#include "systick.h"  // for millis()

#define RANDOM_STRENGTH 2
#include "lib_rand.h"

// you can declare variables for your animation here
#define CONCURRENT_DROPS 4
#define DROPLIFE 1200


int droptime[CONCURRENT_DROPS];
int droplife[CONCURRENT_DROPS];
int dropx[CONCURRENT_DROPS];
int dropy[CONCURRENT_DROPS];

int apsqrt(int i) {
    if (i == 0) return 0;
    int x = 1 << ((32 - __builtin_clz(i)) / 2);
    x = (x + i / x) / 2;
    //	x = (x + i/x)/2; //May be needed depending on how precise you want. (Below graph is without this line)
    return x;
}

static void droplets_init(void) {
    // setup your animation
    seed(SysTick->CNT);
    for (int i = 0; i < CONCURRENT_DROPS; i++) {
        droptime[i] = DROPLIFE * i / CONCURRENT_DROPS;
        dropx[i] = rand() % MATRIX_WIDTH;
        dropy[i] = rand() % MATRIX_HEIGHT;
        droplife[i] = (rand() % 80) + 80;
    }
}

#define MAX_INTENSITY 256

uint8_t fc;

static void droplets_tick() {
    // draw something to matrix

    

    int y, x, i;
    int d;
    uint8_t framebuffer[MATRIX_WIDTH * MATRIX_HEIGHT] = {0};
    uint8_t colbuffer[MATRIX_WIDTH * MATRIX_HEIGHT] = {0};
    

    for (d = 0; d < CONCURRENT_DROPS; d++) {
        i = 0;
        for (y = 0; y < MATRIX_HEIGHT; y++) {
            for (x = 0; x < MATRIX_WIDTH; x++) {
                int dx = x - dropx[d];
                int dy = y - dropy[d];
                int apd = ((apsqrt(dx * dx * MAX_INTENSITY * 48 + dy * dy * MAX_INTENSITY * 48) - droptime[d] * (MAX_INTENSITY / 16))) + MAX_INTENSITY;
                if (apd < 0) apd = -apd;
                int inten = (MAX_INTENSITY - apd - 1);
                if (inten < 0) inten = 0;
                colbuffer[i] = inten > 0 ? (colbuffer[i] / 2) + (d/2) : colbuffer[i];
                inten += framebuffer[i];
                if (inten >= MAX_INTENSITY) inten = MAX_INTENSITY - 1;
                framebuffer[i] = inten;
                i++;
            }
        }

        droptime[d]++;
        droplife[d]--;
        if (droplife[d] < 0) {
            droptime[d] = 0;
            droplife[d] = (rand() % 80) + 80;
            dropx[d] = rand() % MATRIX_WIDTH;
            dropy[d] = rand() % MATRIX_HEIGHT;
        }
    }

    for (x = 0; x < MATRIX_WIDTH; x++)
    for (y = 0; y < MATRIX_HEIGHT; y++) {
        uint8_t h = 128 + (colbuffer[y * MATRIX_WIDTH + x] * (48 / CONCURRENT_DROPS));
        gfxSetPixelMappedRGB(x, y, fromHSV(h,255, framebuffer[y * MATRIX_WIDTH + x]));
    }

    fc++;
}

static const animation_t droplets_animation = {
    .init = droplets_init,
    .tick = droplets_tick,
    .tick_interval = 15,
};

#endif