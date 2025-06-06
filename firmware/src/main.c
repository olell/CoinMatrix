

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

// coin matrix
const uint8_t matrix_pins[16] = {
    CHARLIE_PORT_D | 6,  // P2 L0
    CHARLIE_PORT_D | 5,  // P2 L1
    CHARLIE_PORT_D | 4,  // P2 L2
    CHARLIE_PORT_D | 3,  // P2 L3
    CHARLIE_PORT_D | 2,  // P2 L4
    CHARLIE_PORT_C | 7,  // P4 L5
    CHARLIE_PORT_C | 6,  // P4 L6
    CHARLIE_PORT_C | 5,  // P4 L7
    CHARLIE_PORT_C | 2,  // P4 L8
    CHARLIE_PORT_C | 0,  // P4 L9
    CHARLIE_PORT_C | 1,  // P4 L10
    CHARLIE_PORT_C | 3,  // P4 L11
    CHARLIE_PORT_A | 2,  // P8 L12
    CHARLIE_PORT_A | 1,  // P8 L13
    CHARLIE_PORT_D | 7,  // P2 L14
    CHARLIE_PORT_D | 0,  // P2 L15
};

int main() {
    SystemInit();
    funGpioInitAll();
    systickInit();

    // register animations here
    register_animation(&pulse_animation);

    current_animation_idx = 0;
    current_animation = animations[current_animation_idx];

    charlieSetup(matrix_pins);

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