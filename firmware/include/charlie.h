
#ifndef _CHARLIE_H
#define _CHARLIE_H

#include <stdint.h>

#define CHARLIE_PWM_BITS 4
#define CHARLIE_MAX_BRIGHTNESS 16  // must be 2^CHARLIE_PWM_BITS
#define CHARLIE_PIN_COUNT 16

// display size
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8

// physical matrix size
#define CHARLIE_WIDTH 16
#define CHARLIE_HEIGHT 16

#define CHARLIE_PORT_A (1 << 7)
#define CHARLIE_PORT_C (1 << 6)
#define CHARLIE_PORT_D (1 << 5)

void charlieSetup();
void charlieDisplay();
void charlieSetPixelRaw(int px, uint8_t v);
uint8_t charlieGetPixelRaw(int px);
#endif