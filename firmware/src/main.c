#include <stdint.h>
#include <stdio.h>

#include "animation.h"
#include "animations/animationPlayer.h"
#include "animations/cycle.h"
#include "animations/droplets.h"
#include "animations/firework.h"
#include "animations/matrix.h"
#include "animations/pac.h"
#include "animations/rolling_text.h"
#include "ch32fun.h"
#include "charlie.h"
#include "systick.h"

#define ANIMATION_CYCLE_INTERVAL 20000 // milliseconds between each animation change

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

    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO | RCC_AFIOEN;

    // register animations here
    register_animation(&cycle_animation);
    register_animation(&pac_animation);
    register_animation(&animation_player);
    register_animation(&rolling_text_animation);
    register_animation(&firework_animation);
    register_animation(&matrix_animation);

    current_animation_idx = 0;
    current_animation = animations[current_animation_idx];
    current_animation->init();

    charlieSetup();

    uint32_t anim_start_time = millis();

    for (;;) {
        current_animation->tick();
        Delay_Ms(current_animation->tick_interval);
        if (millis() - anim_start_time > ANIMATION_CYCLE_INTERVAL) {
            anim_start_time = millis();
            if (++current_animation_idx >= animation_count)
                current_animation_idx = 0;
            current_animation = animations[current_animation_idx];
            current_animation->init();
        }
    }
}
