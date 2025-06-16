import sys

# ch32v003 register addresses
GPIOC_OUTDR = 0x4001100C
GPIOD_OUTDR = 0x4001140C

# matrix hardware description
WD = 12 # width
HG = 16 # height

# pins used for charlieplexing, as tuples of ("PORT", PIN). Only GPIO C & D!
PINS = [ 
    ("D", 6),
    ("D", 5),
    ("D", 4),
    ("D", 3),
    ("D", 2),
    ("C", 7),
    ("C", 6),
    ("C", 5),
    ("C", 2),
    ("C", 0),
    ("C", 1),
    ("C", 3),
    ("D", 1),
    ("C", 4),
    ("D", 7),
    ("D", 0),
]

# precalculated config register values
conf_register_values = {
    "C": [],
    "D": []
}
# precalculated output register values
out_register_addresses = []
out_register_values = []

x = 0
x_idx = 1
y = 0
while 1:
    port_x, pin_x = PINS[x_idx]
    port_y, pin_y = PINS[y]

    cfg_x = 2 << (pin_x * 4)
    cfg_y = 2 << (pin_y * 4)

    out_x = 1 << (pin_x)
    out_y = 1 << (pin_y)

    if (port_x != port_y):
        # config register values
        conf_register_values[port_x].append(cfg_x)
        conf_register_values[port_y].append(cfg_y)
    else:
        conf_register_values[port_x].append(cfg_x | cfg_y)
        conf_register_values["C" if port_x == "D" else "D"].append(0)


    # todo: this code assumes the y pins are anodes
    if True:
        if port_y == "C":
            out_register_addresses.append(GPIOC_OUTDR)
        elif port_y == "D":
            out_register_addresses.append(GPIOD_OUTDR)
        out_register_values.append(out_y)
    else:
        if port_x == "C":
            out_register_addresses.append(GPIOC_OUTDR)
        elif port_x == "D":
            out_register_addresses.append(GPIOD_OUTDR)
        out_register_values.append(out_x)
    

    print(f"x, y: {x_idx}, {y}", file=sys.stderr)

    x += 1
    x_idx += 1
    if (x_idx == y): x_idx += 1
    if (x >= WD):
        x = 0
        x_idx = 0
        y += 1
        if (y == HG):
            break


mem_usage = len(conf_register_values["C"])
print(f"#define CHARLIE_CYCLE_COUNT {mem_usage}")
print(f"const uint32_t charlie_gpioc_cfglr[CHARLIE_CYCLE_COUNT] = {{{', '.join(list(map(lambda x: f"0x{x:08x}", conf_register_values["C"])))}}};")
print(f"const uint32_t charlie_gpiod_cfglr[CHARLIE_CYCLE_COUNT] = {{{', '.join(list(map(lambda x: f"0x{x:08x}", conf_register_values["D"])))}}};")
print(f"const uint32_t charlie_out_regs[CHARLIE_CYCLE_COUNT] = {{{', '.join(list(map(lambda x: f"0x{x:08x}", out_register_addresses)))}}};")
print(f"const uint8_t charlie_out_vals[CHARLIE_CYCLE_COUNT] = {{{', '.join(list(map(lambda x: f"0x{x:02x}", out_register_values)))}}};")

print("Memory usage:", file=sys.stderr)
flash = 13 * mem_usage # uint32_t = 4 bytes, uint8_t = 1 byte -> 3 * 4 + 1 = 13
print(f"Flash: {flash} bytes", file=sys.stderr)
print(f"RAM: {mem_usage} bytes", file=sys.stderr)