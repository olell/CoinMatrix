# Specification of ledVM

ledVM is a very minimal bytecode language to build led matrix animations.

## Structure of a program

A program consists of three parts, 1. the header, 2. the data, 3. the code

### The header

The header is 8 bytes long:

| Byte | Description                   |
| ---: | ----------------------------- |
|    1 | Code size, Low                |
|    2 | Code size, High               |
|    3 | Data size, Low                |
|    4 | Data size, High               |
|    5 | Expected matrix width         |
|    6 | Expected matrix height        |
|    7 | tick interval in milliseconds |
|    8 | Flags                         |

_Flags_ byte:

| Bit | Description                            |
| --: | -------------------------------------- |
| ..7 | not used                               |
|   3 | Re-run init before tick                |
|   2 | Clear screen before tick               |
|   1 | Color mode msb, 0: mono, 1: rgb, 3:hsv |
|   0 | Color mode lsb                         |

## Data

After the header there are `data size` bytes of arbitrary data. The data might
be read from the animation using the `rd` operation.

## Code

After the data there may are `code size` bytes of code. The code section repeatedly
consists of `instructions, params, ...` bytes. Each instruction is 1 byte large
and may have a varying amount of parameters depending on the instruction.

There are three instruction types, `operations`, `controls` and `labels` (which
is a special `control`).

A instruction can use `lateral` or `memory` mode for source of calculations or
control flow, the destination is always `memory` mode.

The code has access to 256 (1 byte addr) memory locations that can be used to
store values. Each memory location can also be used as a pointer to another
memory location if the instruction uses `indirect` mode. Every memory location
stores as unsigned 8 bit value.

For each frame the code is executed until the end is reached, therefore it is
necessary that the code wich should be executed each `tick` is the very last code.
The `init` code should be right before the `tick` code, so that the setup can
start there and continue executing until the `tick` label is encountered.

### Instruction byte structure:

| Bit | Description                                                             |
| --: | ----------------------------------------------------------------------- |
|   7 | Instruction type: `0`->`operation`, `1`->`control`                      |
|   6 | lbl; source type: `0`->`lateral`, `1`->`memory`                         |
|   5 | lbl; source memory: `0`->`direct`, `1`->`indirect` if bit 6 is `memory` |
|   4 | lbl; for `operation`: dest memory: `0`->`direct`, `1`->`indirect`       |
|   4 | lbl; for `control`: addr mode: `0`->`lbl`, `1`->`relative`              |
|   3 | lbl; instr                                                              |
|   2 | lbl; instr                                                              |
|   1 | lbl; instr                                                              |
|   0 | instr, if instr type is `control`, `1` sets the type to `label`         |

For `operations` the bits 0..4 set which operation should be executed.
For `controls` the bits 1..3 set which control should be executed.
For `label` the bits 1..7 set the label id (used for `jmp`/`if`)

## `Operations`

| Instruction | Operation, operator | Params                     |
| ----------: | ------------------- | -------------------------- |
|           1 | `add` `+`           | `dest`, `src`              |
|           2 | `sub` `-`           | `dest`, `src`              |
|           3 | `mul` `*`           | `dest`, `src`              |
|           4 | `div` `/`           | `dest`, `src`              |
|           5 | `mod` `%`           | `dest`, `src`              |
|           6 | `shl` `<<`          | `dest`, `src`              |
|           7 | `shr` `>>`          | `dest`, `src`              |
|           8 | `and` `&`           | `dest`, `src`              |
|           9 | `or` `\|`           | `dest`, `src`              |
|           A | `not` `~`           | `dest`, `src`              |
|           B | `ld` `=`            | `dest`, `src`              |
|           C | `rd` `:=`           | `dest`, `src_lo`, `src_hi` |
|           D | `eq` `==`           | `dest`, `src`              |

- for most operations the execution is done as:
  - `dest = dest (op) source` or
  - `dest = (op) source` for `not`
- for `ld` it is:
  - `dest = source`
- for `rd` it is:
  - `dest = data[source_lo | (source_hi << 8)]`

## `Controls`

| Instruction | Operation | Params            |
| ----------: | --------- | ----------------- |
|           1 | goto      | `label` or offset |
|           2 | if        | `label` or offset |
|           3 | jmp       | `label` or offset |
|           4 | ret       | -                 |
|           5 | setpx     | -                 |
|           6 | getpx     | -                 |

- `goto` jumps to the `label` or offset
- `if` does so if the last operation resulted in a non-zero value
- `jmp` jumps to the `label` or offset and allows to return
- `ret` returns back to the point where the last `jmp` came from
- `setpx` / `getpx` set or get pixel values, see below for params

## `Labels`

Each label can be 6 bits large, there are some reserved labels

- `0` -> `init`
- `1` -> `tick`

## Memory map

Most of the memory addresses can be used to store abitrary values in RAM, but
there are some values the are mapped and can only be read. Writing to those
locations results in no operation

- `0xff` -> a random value between 0..255
- `0xfe` -> the actual matrix height (>= expected from header otherwise the code wont run)
- `0xfd` -> the actual matrix width (>= expected from hedaer otherwise the code wont run)
- `0xfc` -> a frame counter, increases each tick, overflows every `@0xfb` frames

And some values that are read/write but reserved for special purposes

- `0xfb` -> the value at which the framecounter should wrap around (Defaults to 0xff)
- `0xfa` -> tick interval override, if `0` the value from the header is used
- `0xf9` -> color blue or hsv value
- `0xf8` -> color green or hsv saturation
- `0xf7` -> monochrome value, color red or hsv hue
- `0xf6` -> y
- `0xf5` -> x

`0xf5`..`0xf9` are used as params if `setpx` is called. If `getpx` is used,
`0xf5` and `0xf6` are used as param and `0xf7`..`0xf9` contain the result
