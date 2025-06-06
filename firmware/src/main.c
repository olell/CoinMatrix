

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "animation.h"
#include "animations/pulse.h"
#include "ch32fun.h"
#include "charlie.h"
#include "systick.h"

static const animation_t* animations[MAX_ANIMATIONS];
static size_t animation_count = 0;
size_t current_animation_idx = 0;
const animation_t* current_animation;

void register_animation(const animation_t* anim) {
    if (animation_count < MAX_ANIMATIONS) {
        animations[animation_count++] = anim;
    }
}

const animation_t* get_animation(size_t index) {
    if (index >= animation_count) return NULL;
    return animations[index];
}

int main() {
    SystemInit();
    funGpioInitAll();
    systickInit();

    // register animations here
    register_animation(&pulse_animation);

    current_animation_idx = 0;
    current_animation = animations[current_animation_idx];

    charlieSetup();

    uint32_t anim_start_time = millis();

    for (;;) {
        current_animation->tick();
        Delay_Ms(current_animation->tick_interval);

        if (millis() - anim_start_time > 5000) {
            anim_start_time = millis();
            if (++current_animation_idx == animation_count)
                current_animation_idx = 0;
        }
    }
}