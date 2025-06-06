#ifndef _ANIMATION_H
#define _ANIMATION_H

#include <stdint.h>

typedef void (*animation_init_fn)(void);
typedef void (*animation_tick_fn)(void);

typedef struct {
    animation_init_fn init;
    animation_tick_fn tick;
    uint16_t tick_interval;
} animation_t;

// Max number of animations
#define MAX_ANIMATIONS 16

#endif