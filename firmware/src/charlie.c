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

// matrix pinout
// coin matrix
const uint8_t charlie_pins[16] = {
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

#define GPIOA_CFG GPIOA->CFGLR
#define GPIOA_OUT GPIOA->OUTDR
#define GPIOC_CFG GPIOC->CFGLR
#define GPIOC_OUT GPIOC->OUTDR
#define GPIOD_CFG GPIOD->CFGLR
#define GPIOD_OUT GPIOD->OUTDR

uint_fast32_t raw_pixels[CHARLIE_PIN_COUNT * CHARLIE_PIN_COUNT];

volatile uint32_t* conf_reg[CHARLIE_PIN_COUNT];
uint32_t conf_reg_clean[CHARLIE_PIN_COUNT];
volatile uint32_t* out_registers[CHARLIE_PIN_COUNT];

uint_fast32_t conf_precalc[CHARLIE_PIN_COUNT];
uint_fast32_t out_precalc[CHARLIE_PIN_COUNT];

uint_fast32_t row;
uint_fast32_t col;
uint_fast32_t pwm;

void charlieSetPixelXY(uint8_t x, uint8_t y, uint8_t v) {
    uint8_t c = (x >= y) ? x + 1 : x;
    raw_pixels[y * CHARLIE_PIN_COUNT + c] = v;
}

void charlieSetPixelRGB(uint8_t x, uint8_t y, uint32_t color) {
    charlieSetPixelXY((x * 3), y, ((color >> 16) & 0xff));
    charlieSetPixelXY((x * 3) + 1, y, ((color >> 8) & 0xff));
    charlieSetPixelXY((x * 3) + 2, y, (color & 0xff));
}

void charlieSetPixelMappedRGB(uint8_t x, uint8_t y, uint32_t color) {
    if ((x == 0 || x == 7) && (y == 0 || y == 7)) return;
    if (x > 7 || y > 7) return;

    uint8_t rw = y;
    uint8_t cl = x;
    if (cl > 3) {
        cl -= 4;
        rw += 8;
    }

    charlieSetPixelRGB(cl, rw, color);
}

void charlieSetPixelRaw(int px, uint8_t v) {
    raw_pixels[px] = v;
}

void charlieSetup() {
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
    const register uint32_t out_val asm("a1") = out_precalc[col];
    volatile register uint32_t* out_reg asm("a2") = out_registers[col];
#else
    const register uint32_t out_val asm("a1") = out_precalc[row];
    volatile register uint32_t* out_reg asm("a2") = out_registers[row];
#endif

    const register uint32_t pix_val asm("a3") = raw_pixels[row * CHARLIE_WIDTH + col];

    for (pwm = 0; pwm <= CHARLIE_MAX_BRIGHTNESS; pwm++) {
        *(out_reg) = (pwm < pix_val) ? out_val : 0;
    }

    *(conf_reg[row]) = conf_reg_clean[row];
    *(conf_reg[col]) = conf_reg_clean[col];

    // increase row/column, skip row=column
    col++;
    if (col == row) {
        col++;
    }
    if (col >= CHARLIE_WIDTH) {
        col = 0;
        if (++row == CHARLIE_HEIGHT) row = 0;
    }
}