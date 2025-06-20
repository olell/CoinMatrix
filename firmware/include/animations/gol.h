#ifndef _ANIM_GOL_H
#define _ANIM_GOL_H

#include "lib_rand.h"
#include "animation.h"  // for animation_t
#include "gfx.h"

uint8_t gol_fb[MATRIX_WIDTH * MATRIX_HEIGHT] = {0};
uint8_t gol_fb_copy[MATRIX_WIDTH * MATRIX_HEIGHT] = {0};
uint8_t gol_fb_copy2[MATRIX_WIDTH * MATRIX_HEIGHT] = {0};

static void gol_store2() {
    for (int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
        gol_fb_copy2[i] = gol_fb_copy[i];
    }
}
static void gol_store() {
    for (int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
        gol_fb_copy[i] = gol_fb[i];
    }
}

static void gol_init(void) {
    // setup your animation
    seed(SysTick->CNT);
    for (int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
        gol_fb[i] = rand() > 0x80000000 ? 0 : 1;  // rand
    }
}

int wrap(int x, int limit) {
    if (x < 0) return limit - 1;
    if (x >= limit) return 0;
    return x;
}

int gol_fc = 0;
int gol_keep_countdown = 0;

static void gol_tick() {
    if (++gol_fc == 255) gol_fc = 0;
    gol_store2();
    gol_store();
    for (int y = 0; y < MATRIX_HEIGHT; y++)
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            int neigh = 0;
            neigh += gol_fb_copy[wrap((y - 1), MATRIX_HEIGHT) * MATRIX_WIDTH + wrap((x - 1), MATRIX_WIDTH)] > 0 ? 1 : 0;
            neigh += gol_fb_copy[wrap((y - 1), MATRIX_HEIGHT) * MATRIX_WIDTH + wrap((x), MATRIX_WIDTH)] > 0 ? 1 : 0;
            neigh += gol_fb_copy[wrap((y - 1), MATRIX_HEIGHT) * MATRIX_WIDTH + wrap((x + 1), MATRIX_WIDTH)] > 0 ? 1 : 0;
            neigh += gol_fb_copy[wrap((y), MATRIX_HEIGHT) * MATRIX_WIDTH + wrap((x - 1), MATRIX_WIDTH)] > 0 ? 1 : 0;
            neigh += gol_fb_copy[wrap((y), MATRIX_HEIGHT) * MATRIX_WIDTH + wrap((x + 1), MATRIX_WIDTH)] > 0 ? 1 : 0;
            neigh += gol_fb_copy[wrap((y + 1), MATRIX_HEIGHT) * MATRIX_WIDTH + wrap((x - 1), MATRIX_WIDTH)] > 0 ? 1 : 0;
            neigh += gol_fb_copy[wrap((y + 1), MATRIX_HEIGHT) * MATRIX_WIDTH + wrap((x), MATRIX_WIDTH)] > 0 ? 1 : 0;
            neigh += gol_fb_copy[wrap((y + 1), MATRIX_HEIGHT) * MATRIX_WIDTH + wrap((x + 1), MATRIX_WIDTH)] > 0 ? 1 : 0;

            if (neigh < 2 || neigh > 3) {
                gol_fb[y * MATRIX_WIDTH + x] = 0;
            } else if (neigh == 3) {
                gol_fb[y * MATRIX_WIDTH + x] = 1;
            } else {
                gol_fb[y * MATRIX_WIDTH + x] = gol_fb[y * MATRIX_WIDTH + x] > 0 ? gol_fb[y * MATRIX_WIDTH + x] + 1 : 0;
            }
        }

    // compare
    int eq = 0;
    int eq2 = 0;
    for (int i = 0; i < MATRIX_WIDTH * MATRIX_HEIGHT; i++) {
        if ((gol_fb[i] > 0 && gol_fb_copy[i] > 0) || (gol_fb[i] == 0 && gol_fb_copy[i] == 0)) {
            eq += 1;
        }
        if ((gol_fb[i] > 0 && gol_fb_copy2[i] > 0) || (gol_fb[i] == 0 && gol_fb_copy2[i] == 0)) {
            eq2 += 1;
        }
    }
    if (eq == MATRIX_WIDTH * MATRIX_HEIGHT || eq2 == MATRIX_WIDTH * MATRIX_HEIGHT) {
        if (gol_keep_countdown == 0)
            gol_keep_countdown = 10;
    }
    if (gol_keep_countdown > 0) {
        if (--gol_keep_countdown == 0) gol_init();
    }

    for (int y = 0; y < MATRIX_HEIGHT; y++)
        for (int x = 0; x < MATRIX_WIDTH; x++) {
            gfxSetPixelMappedRGB(x, y, fromHSV(gol_fb[y * MATRIX_WIDTH + x] * 16 + gol_fc, 255, gol_fb[y * MATRIX_WIDTH + x] > 0 ? 255 : 0));
        }
}

static const animation_t gol_animation = {
    .init = gol_init,
    .tick = gol_tick,
    .tick_interval = 175,
};

#endif