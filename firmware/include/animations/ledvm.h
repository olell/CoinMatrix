#ifndef _ANIM_LEDVM_H
#define _ANIM_LEDVM_H

#include <stdint.h>
#ifdef LEDVM_DEBUG
#include <stdio.h>
#endif

#include "animation.h"  // for animation_t
#include "ch32fun.h"
#include "charlie.h"  // for matrix function
#include "gfx.h"
#include "systick.h"  // for millis()
#define RANDOM_STRENGTH 2
#include "lib_rand.h"

// instruction decoding
#define LEDVM_OPERATION 0
#define LEDVM_CONTROL 1

#define LEDVM_SOURCE_TYPE_LATERAL 0
#define LEDVM_SOURCE_TYPE_MEMORY 1
#define LEDVM_MEMORY_DIRECT 0
#define LEDVM_MEMORY_INDIRECT 1

#define LEDVM_LABEL 0
#define LEDVM_RELATIVE 1

// predefined labels
#define LEDVM_LABEL_INIT 0
#define LEDVM_LABEL_TICK 1

// color modes
#define LEDVM_COLORMODE_MONO 0
#define LEDVM_COLORMODE_RGB 1
#define LEDVM_COLORMODE_HSV 3

// memory mapped locations
#define LEDVM_MAP_RAND 0xFF
#define LEDVM_MAP_ACTUAL_HEIGHT 0xFE
#define LEDVM_MAP_ACTUAL_WIDTH 0xFD
#define LEDVM_MAP_FC 0xFC
#define LEDVM_MAP_FC_WRAP 0xFB
#define LEDVM_MAP_TICK_INTERVAL 0xFA
#define LEDVM_MAP_COLOR_B_V 0xF9
#define LEDVM_MAP_COLOR_G_S 0xF8
#define LEDVM_MAP_COLOR_V_R_H 0xF7
#define LEDVM_MAP_Y 0xF6
#define LEDVM_MAP_X 0xF5

// opcodes operations
#define LEDVM_ADD 0x01
#define LEDVM_SUB 0x02
#define LEDVM_MUL 0x03
#define LEDVM_DIV 0x04
#define LEDVM_MOD 0x05
#define LEDVM_SHL 0x06
#define LEDVM_SHR 0x07
#define LEDVM_AND 0x08
#define LEDVM_OR 0x09
#define LEDVM_NOT 0x0A
#define LEDVM_LD 0x0B
#define LEDVM_RD 0x0C
#define LEDVM_EQ 0x0D

// opcodes controls
#define LEDVM_GOTO 0x01
#define LEDVM_IF 0x02
#define LEDVM_JMP 0x03
#define LEDVM_RET 0x04
#define LEDVM_SETPX 0x05
#define LEDVM_GETPX 0x06

//
typedef struct LEDVMHeader {
    // bytes 1..2
    union {
        uint16_t code_size;
        struct {
            uint8_t code_size_lo;
            uint8_t code_size_hi;
        };
    };
    // bytes 3..4
    union {
        uint16_t data_size;
        struct {
            uint8_t data_size_lo;
            uint8_t data_size_hi;
        };
    };
    // byte 5..7
    uint8_t matrix_width;
    uint8_t matrix_height;
    uint8_t tick_interval;
    // flags
    union {
        uint8_t flags;
        struct {
            uint8_t color_mode : 2;
            uint8_t clear_before_tick : 1;
            uint8_t init_before_tick : 1;
            uint8_t reserved : 4;
        };
    };
} ledvm_header_t;

typedef struct LEDVMInstruction {
    union {
        uint8_t instruction;
        struct {
            uint8_t _reserved : 7;
            uint8_t instruction_type : 1;
        };
        struct {
            uint8_t instr : 4;
            uint8_t dest_memory : 1;
            uint8_t source_memory : 1;
            uint8_t source_type : 1;
            uint8_t instruction_type : 1;
        } operation;
        struct {
            uint8_t is_label : 1;
            uint8_t instr : 3;
            uint8_t addr_mode : 1;
            uint8_t source_memory : 1;
            uint8_t source_type : 1;
            uint8_t instruction_type : 1;
        } control;
    };
} ledvm_instruction_t;

const uint8_t program[] = {
    0x35,
    0x00,
    0x00,
    0x00,
    0x08,
    0x08,
    0x64,
    0x01,
    0x81,
    0x0b,
    0xf7,
    0xff,
    0x0b,
    0xf8,
    0x00,
    0x0b,
    0xf9,
    0x20,
    0x0b,
    0xf6,
    0x00,
    0x0b,
    0xf5,
    0x01,
    0x83,
    0x4b,
    0x00,
    0xf7,
    0x0d,
    0x00,
    0x00,
    0x84,
    0x02,
    0x01,
    0xf6,
    0x01,
    0x45,
    0xf6,
    0xfe,
    0x4b,
    0x00,
    0xf6,
    0x84,
    0x02,
    0x01,
    0xf5,
    0x01,
    0x45,
    0xf5,
    0xfd,
    0x85,
    0x8a,
    0x4a,
    0xf7,
    0xf7,
    0x4a,
    0xf8,
    0xf8,
    0x4a,
    0xf9,
    0xf9,
};
static uint8_t *ledvm_program;
static uint8_t ledvm_memory[256];

static uint8_t is_invalid;

static ledvm_header_t *ledvm_header;

static uint16_t ledvm_pc;
static uint16_t ledvm_ret;
static uint16_t ledvm_lbls[64];
static uint8_t ledvm_fc;
static uint8_t ledvm_if_cond;

static uint16_t ledvm_code_location;

static void ledvm_write_memory(const uint8_t addr, const uint8_t value) {
    ledvm_memory[addr] = value;
}

static uint8_t ledvm_read_memory(const uint8_t addr) {
    // mapped addresses
    if (addr == LEDVM_MAP_RAND) return 0;  // rand() & 0xff;
    if (addr == LEDVM_MAP_ACTUAL_HEIGHT) return MATRIX_HEIGHT;
    if (addr == LEDVM_MAP_ACTUAL_WIDTH) return MATRIX_WIDTH;
    if (addr == LEDVM_MAP_FC) return ledvm_fc;

    // lower memory
    return ledvm_memory[addr];
}

static uint8_t ledvm_get_addr(const uint8_t addr, const uint8_t mode) {
    if (mode == LEDVM_MEMORY_DIRECT)
        return addr;
    else {
        return ledvm_read_memory(addr);
    }
}

static uint8_t ledvm_get_value(const uint8_t addr, const uint8_t mode) {
    return ledvm_read_memory(ledvm_get_addr(addr, mode));
}

static uint8_t ledvm_get_instr_size(const ledvm_instruction_t *instr) {
    if (instr->instruction_type == LEDVM_OPERATION) {
        // all operations are 3 bytes except `rd` is 4
        return ((instr->operation.instr) == LEDVM_RD) ? 4 : 3;
    }

    // labels are 1 byte
    if (instr->control.is_label) return 1;

    // controls
    switch (instr->control.instr) {
        case LEDVM_GOTO:
        case LEDVM_IF:
        case LEDVM_JMP:
            return 2;
        default:
            return 1;
    }
}

static uint8_t ledvm_instr_source_value(const ledvm_instruction_t *instr, const uint8_t param_offset) {
    if (instr->operation.source_type == LEDVM_SOURCE_TYPE_MEMORY) {
        return ledvm_get_value(
            ledvm_program[ledvm_pc + param_offset],
            instr->operation.source_memory);
    } else {
        return ledvm_program[ledvm_pc + param_offset];
    }
}

static void ledvm_execute_operation(const ledvm_instruction_t *instr) {
    const uint8_t dest_addr = ledvm_get_addr(
        ledvm_program[ledvm_pc + 1], instr->operation.dest_memory);
    const uint8_t src_lo = ledvm_instr_source_value(instr, 2);

    if (instr->operation.instr == LEDVM_RD) {
        const uint8_t src_hi = ledvm_instr_source_value(instr, 3);

        const uint16_t src = src_lo | (src_hi << 8);
        // err: invalid read location
        if (src >= ledvm_header->data_size) return;

        const uint8_t value = ledvm_program[sizeof(ledvm_header_t) + src];
        ledvm_if_cond = value != 0 ? 1 : 0;
        // exec: dest = data[src]
        ledvm_write_memory(dest_addr, value);
        return;
    }
    const uint8_t dest_value = ledvm_read_memory(dest_addr);
    uint8_t result;
    switch (instr->operation.instr) {
        case LEDVM_ADD:
            result = dest_value + src_lo;
            break;
        case LEDVM_SUB:
            result = dest_value - src_lo;
            break;
        case LEDVM_MUL:
            result = dest_value * src_lo;
            break;
        case LEDVM_DIV:
            result = dest_value / src_lo;
            break;
        case LEDVM_MOD:
            result = dest_value % src_lo;
            break;
        case LEDVM_SHL:
            result = dest_value << src_lo;
            break;
        case LEDVM_SHR:
            result = dest_value >> src_lo;
            break;
        case LEDVM_AND:
            result = dest_value & src_lo;
            break;
        case LEDVM_OR:
            result = dest_value | src_lo;
            break;
        case LEDVM_NOT:
            result = ~src_lo;
            break;
        case LEDVM_LD:
            result = src_lo;
            break;
        case LEDVM_EQ:
            result = (dest_value == src_lo) ? 1 : 0;
            break;
    }

    ledvm_if_cond = result != 0 ? 1 : 0;

    // exec: dest = dest (op) source
    ledvm_write_memory(dest_addr, result);
}

static void ledvm_set_pixel() {
    const uint8_t x = ledvm_read_memory(LEDVM_MAP_X);
    const uint8_t y = ledvm_read_memory(LEDVM_MAP_Y);
    if (ledvm_header->color_mode == LEDVM_COLORMODE_MONO) {
        const uint8_t v = ledvm_read_memory(LEDVM_MAP_COLOR_V_R_H);
        gfxSetPixelMappedRGB(x, y, fromRGB8(v, v, v));
    } else {
        const uint8_t r_h = ledvm_read_memory(LEDVM_MAP_COLOR_V_R_H);
        const uint8_t g_s = ledvm_read_memory(LEDVM_MAP_COLOR_G_S);
        const uint8_t b_v = ledvm_read_memory(LEDVM_MAP_COLOR_V_R_H);
        gfxSetPixelMappedRGB(x, y,
                             (ledvm_header->color_mode == LEDVM_COLORMODE_RGB) ? fromRGB8(r_h, g_s, b_v) : fromHSV(r_h, g_s, b_v));
    }
}

static void ledvm_get_pixel() {
    // todo
}

static void ledvm_execute_control(const ledvm_instruction_t *instr) {
    if (instr->control.instr == LEDVM_RET) {
        ledvm_pc = ledvm_ret - 1;  // -1 to compensate for next_instr: increase
        return;
    }
    if (instr->control.instr == LEDVM_SETPX) {
        ledvm_set_pixel();
#ifdef LEDVM_DEBUG
        printf("LEDVM setpx \n");
#endif
        return;
    }
    if (instr->control.instr == LEDVM_GETPX) {
        ledvm_get_pixel();
#ifdef LEDVM_DEBUG
        printf("LEDVM getpx \n");
#endif
        return;
    }

    uint16_t new_pc = ledvm_pc;

    uint8_t lbl_rel = ledvm_program[ledvm_pc + 1];  // lateral?
    if (instr->control.source_type == LEDVM_SOURCE_TYPE_MEMORY) {
        lbl_rel = ledvm_read_memory(lbl_rel);  // direct memory?
        if (instr->control.source_memory == LEDVM_MEMORY_INDIRECT) {
            lbl_rel = ledvm_read_memory(lbl_rel);  // indirect memory?
        }
    }

    if (instr->control.addr_mode == LEDVM_LABEL) {
        new_pc = ledvm_lbls[lbl_rel] - 2;  // -2 to compensate next_instr: increase
    } else {                               // relative: todo: negative
        new_pc += lbl_rel - 2;             // -2 to compensate next_instr: increase
    }

    switch (instr->control.instr) {
        case LEDVM_GOTO:
            ledvm_pc = new_pc;
            break;
        case LEDVM_IF:
            ledvm_pc = ledvm_if_cond ? new_pc : ledvm_pc;
            break;
        case LEDVM_JMP:
            ledvm_ret = ledvm_pc;
            ledvm_pc = new_pc;
            break;
    }
}

static void ledvm_execute_range(uint16_t start, uint16_t end) {
#ifdef LEDVM_DEBUG
    printf("LEDVM executing 0x%04x..0x%04x\n", start, end);
#endif
    ledvm_pc = start;
    while (ledvm_pc < end) {
        const ledvm_instruction_t *instr = (const ledvm_instruction_t *)ledvm_program + ledvm_pc;

#ifdef LEDVM_DEBUG
        printf("LEDVM PC: 0x%04x INSTR: %02x ", ledvm_pc, instr->instruction);
#endif

        // skip labels in execution
        if (instr->instruction_type == LEDVM_CONTROL &&
            instr->control.is_label) {
#ifdef LEDVM_DEBUG
            printf("LB \n");
#endif
            goto next_instr;
        }

        if (instr->instruction_type == LEDVM_OPERATION) {
#ifdef LEDVM_DEBUG
            printf("OP \n");
#endif
            ledvm_execute_operation(instr);
        } else {
#ifdef LEDVM_DEBUG
            printf("CT \n");
#endif
            ledvm_execute_control(instr);
        }

    next_instr:
        ledvm_pc += ledvm_get_instr_size(instr);
    }
}

static void ledvm_init(void) {
#ifdef LEDVM_DEBUG
    printf("LEDVM Init\n");
#endif

    // todo load program from "somewhere"TM
    ledvm_program = (uint8_t *)program;

    // parse header
    ledvm_header = (ledvm_header_t *)ledvm_program;

    // animation is for bigger matrices
    if (ledvm_header->matrix_height > MATRIX_HEIGHT ||
        ledvm_header->matrix_width > MATRIX_WIDTH) {
        is_invalid = 1;
        return;
    }

    ledvm_code_location = sizeof(ledvm_header_t) + ledvm_header->data_size;

#ifdef LEDVM_DEBUG
    printf("LEDVM code location: %d\n", ledvm_code_location);
    printf("LEDVM code size:     %d\n", ledvm_header->code_size);
#endif

    // clear memory
    for (int i = 0; i < sizeof(ledvm_memory); i++) {
        ledvm_memory[i] = 0;
    }

    // set default values for special purpose memory addrs
    ledvm_memory[LEDVM_MAP_FC_WRAP] = 0xff;

    // discover labels
    uint16_t p = ledvm_code_location;
    while (p < ledvm_code_location + ledvm_header->code_size) {
        const ledvm_instruction_t *instr = (const ledvm_instruction_t *)ledvm_program + p;

        // first and last bit set are labels
        if (instr->instruction_type == LEDVM_CONTROL &&
            instr->control.is_label) {
            const uint8_t id = (instr->instruction & 0x7e) >> 1;
            ledvm_lbls[id] = p;

#ifdef LEDVM_DEBUG
            printf("LEDVM label: 0x%02x @ 0x%04x\n", id, p);
#endif
        }
        p += ledvm_get_instr_size(instr);
    }

#ifdef LEDVM_DEBUG
    printf("LEDVM discovered labels\n");
#endif
    // start @ init
    ledvm_execute_range(ledvm_lbls[LEDVM_LABEL_INIT], ledvm_lbls[LEDVM_LABEL_TICK]);
}

static void ledvm_tick() {
    if (is_invalid) return;

    ledvm_execute_range(ledvm_lbls[LEDVM_LABEL_TICK], ledvm_code_location + ledvm_header->code_size);

    if (++ledvm_fc == ledvm_memory[LEDVM_MAP_FC_WRAP]) ledvm_fc = 0;

    const uint8_t interval_memory = ledvm_read_memory(LEDVM_MAP_TICK_INTERVAL);
    if (interval_memory)
        // msleep(interval_memory);
        Delay_Ms(interval_memory);
    else
        // msleep(ledvm_header->tick_interval);
        Delay_Ms(ledvm_header->tick_interval);
}

static const animation_t ledvm_animation = {
    .init = ledvm_init,
    .tick = ledvm_tick,
    .tick_interval = 0,
};

#endif