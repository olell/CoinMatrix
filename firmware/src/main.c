

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ch32fun.h"
#include "charlie.h"

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
#ifndef RUN_LOCAL
    SystemInit();
    funGpioInitAll();
#endif

    charlieSetup(matrix_pins);
    printf("First run foo!\n");

    int c = 0;
    int l = 0;

    uint8_t x = 0;
    uint8_t y = 0;

    for (uint8_t y = 0; y < 8; y++) {
        for (uint8_t x = 0; x < 8; x++) {
        }
    }

    for (;;) {
        charlieDisplay();
        if (++c == 10000) {
            c = 0;
            charlieSetPixelMappedRGB(x, y, 0, 0, 0);
            if (++x == 8) {
                x = 0;
                if (++y == 8) y = 0;
            }
            charlieSetPixelMappedRGB(x, y, 15, 15, 15);
        }
    }
}