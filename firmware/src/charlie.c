/**
 * matrix structure
 *
 *   0 1 2 3 4 ...
 * 0 x
 * 1   x
 * 2     x
 * 3       x
 * 4         x
 * ...
 */

#include "charlie.h"

#include "ch32fun.h"
#include "stdio.h"

#define GPIOA_CFG GPIOA->CFGLR
#define GPIOA_OUT GPIOA->OUTDR
#define GPIOC_CFG GPIOC->CFGLR
#define GPIOC_OUT GPIOC->OUTDR
#define GPIOD_CFG GPIOD->CFGLR
#define GPIOD_OUT GPIOD->OUTDR

uint8_t raw_pixels[CHARLIE_PIN_COUNT * CHARLIE_PIN_COUNT];

volatile uint32_t* conf_reg[CHARLIE_PIN_COUNT];
uint32_t conf_reg_clean[CHARLIE_PIN_COUNT];
volatile uint32_t* out_registers[CHARLIE_PIN_COUNT];

uint32_t conf_precalc[CHARLIE_PIN_COUNT];
uint32_t out_precalc[CHARLIE_PIN_COUNT];

const uint8_t* debug_pins;

uint8_t row;
uint8_t col;
uint8_t pwm;

void charlieSetPixelXY(uint8_t x, uint8_t y, uint8_t v) {
    uint8_t c = (x >= y) ? x + 1 : x;
    raw_pixels[y * CHARLIE_PIN_COUNT + c] = v;
}

void charlieSetPixelRGB(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    charlieSetPixelXY((x * 3), y, r);
    charlieSetPixelXY((x * 3) + 1, y, g);
    charlieSetPixelXY((x * 3) + 2, y, b);
}

void charlieSetPixelMappedRGB(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    if ((x == 0 || x == 7) && (y == 0 || y == 7)) return;
    if (x > 7 || y > 7) return;

    uint8_t rw = y;
    uint8_t cl = x;
    if (cl > 3) {
        cl -= 4;
        rw += 8;
    }

    charlieSetPixelRGB(cl, rw, r, g, b);
}

void charlieSetPixelRaw(int px, uint8_t v) {
    raw_pixels[px] = v;
}

void charlieSetup(const uint8_t* charlie_pins) {
    debug_pins = charlie_pins;

    // zero all cfg registers for used pins
    for (uint8_t i = 0; i < CHARLIE_PIN_COUNT; i++) {
        uint8_t pin = charlie_pins[i];
        if ((pin & 0xe0) == CHARLIE_PORT_A) {
            GPIOA_CFG &= ~((uint32_t)0xf << (4 * (pin & 0x1f)));
        } else if ((pin & 0xe0) == CHARLIE_PORT_C) {
            GPIOC_CFG &= ~((uint32_t)0xf << (4 * (pin & 0x1f)));
        } else if ((pin & 0xe0) == CHARLIE_PORT_D) {
            GPIOD_CFG &= ~((uint32_t)0xf << (4 * (pin & 0x1f)));
        }
    }

    row = 0;
    col = 1;
    for (uint8_t i = 0; i < CHARLIE_PIN_COUNT; i++) {
        uint8_t pin = charlie_pins[i];
        if ((pin & 0xe0) == CHARLIE_PORT_A) {
            conf_reg[i] = &GPIOA_CFG;
            conf_reg_clean[i] = GPIOA_CFG;
            out_registers[i] = &GPIOA_OUT;
        } else if ((pin & 0xe0) == CHARLIE_PORT_C) {
            conf_reg[i] = &GPIOC_CFG;
            conf_reg_clean[i] = GPIOC_CFG;
            out_registers[i] = &GPIOC_OUT;
        } else if ((pin & 0xe0) == CHARLIE_PORT_D) {
            conf_reg[i] = &GPIOD_CFG;
            conf_reg_clean[i] = GPIOD_CFG;
            out_registers[i] = &GPIOD_OUT;
        }
        conf_precalc[i] = (uint32_t)2 << (4 * (pin & 0x1f));
        out_precalc[i] = 1 << (pin & 0x1f);
    }
}

inline void charlieDisplay() __attribute__((section(".srodata"))) __attribute__((used)) __attribute__((always_inline));
inline void charlieDisplay() {
    *(conf_reg[row]) |= conf_precalc[row];
    *(conf_reg[col]) |= conf_precalc[col];

#ifdef CHARLIE_COL_IS_ANODE
    const uint8_t out_val = out_precalc[col];
    volatile uint32_t* out_reg = out_registers[col];
#else
    const uint32_t out_val = out_precalc[row];
    volatile uint32_t* out_reg = out_registers[row];
#endif

    const uint8_t pix_val = raw_pixels[row * CHARLIE_WIDTH + col];

    for (pwm = 0; pwm <= CHARLIE_MAX_BRIGHTNESS; pwm++) {
        *(out_reg) = (pwm < pix_val) ? out_val : 0;
    }

    *(conf_reg[row]) = conf_reg_clean[row];
    *(conf_reg[col]) = conf_reg_clean[col];

    // increase row/column, skip row=column
    col++;
    if (col == row) col++;
    if (col >= CHARLIE_WIDTH) {
        col = 0;
        if (++row == CHARLIE_HEIGHT) row = 0;
    }
}