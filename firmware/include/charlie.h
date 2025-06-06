
#ifndef _CHARLIE_H
#define _CHARLIE_H

#include <stdint.h>

#define CHARLIE_MAX_BRIGHTNESS 16
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

void charlieSetup(const uint8_t* charlie_pins);
void charlieDisplay();
void charlieSetPixelRaw(int px, uint8_t v);
void charlieSetPixelXY(uint8_t x, uint8_t y, uint8_t v);
void charlieSetPixelRGB(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void charlieSetPixelMappedRGB(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);

#endif