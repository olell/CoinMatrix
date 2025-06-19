#!/usr/bin/env python3

from PIL import Image
import numpy as np
from argparse import ArgumentParser

OUTPUT = """
#ifndef _GIF_H
#define _GIF_H
#define GIF_LENGTH {length}
static const uint8_t GIF_INSTRUCTIONS[] = {{{instructions}}};
#endif //_GIF_H
"""

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("input")
    parser.add_argument("--background", default="000000")
    parser.add_argument("--bgtresh", default=1, type=int)
    
    args = parser.parse_args()

    r_background = int(args.background[0:2], 16)
    g_background = int(args.background[2:4], 16)
    b_background = int(args.background[4:6], 16)


    im = Image.open(args.input)
    last_frame = np.zeros((8,8,3))

    instructions = list()

    # x, y, rgb
    while True:

        frame = np.asarray(im.resize((8,8)).convert('RGB'))

        for yindex, x in enumerate(frame):
            for xindex, c in enumerate(x):
                r, g, b = list(map(int, c))

                if (c == last_frame[yindex,xindex]).all():
                    continue

                if ((r > r_background - args.bgtresh and r < r_background + args.bgtresh)
                and (g > g_background - args.bgtresh and g < g_background + args.bgtresh)
                and (b > b_background - args.bgtresh and b < b_background + args.bgtresh)):
                    r = 0
                    b = 0
                    g = 0

                # call protocol function
                instructions.append(
                    ((yindex & 7) << 3) | (xindex & 7)
                )
                instructions.append(
                    (r >> 4) & 0xf
                )
                instructions.append(
                    (g & 0xf0) | ((b >> 4) & 0xf)
                )
        
        last_frame = frame
        instructions[-3] |= 0xc0

        # seek till next frame if available
        try:
            im.seek(im.tell()+1)
        except EOFError:
            break

    print(OUTPUT.format(
        length=len(instructions),
        instructions=", ".join(list(map(str, instructions)))
    ))