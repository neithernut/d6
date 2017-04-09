/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Julian Ganz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <alloca.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

/**
 * Dice data
 *
 *     #######
 *     #0 1 2#
 *     #3 4 5#
 *     #6 7 8#
 *     #######
 *
 */
const uint64_t pips =
    ((uint64_t) 0x010) << (1*9) |
    ((uint64_t) 0x044) << (2*9) |
    ((uint64_t) 0x054) << (3*9) |
    ((uint64_t) 0x145) << (4*9) |
    ((uint64_t) 0x155) << (5*9) |
    ((uint64_t) 0x16D) << (6*9);


const size_t dice_row_len = 16;


void
set_row_pixels(
    char* dest,
    uint8_t row,
    uint8_t value
) {
    memset(dest, '#', 14);

    switch (row) {
    case 1:
    case 3:
    case 5:
        {
            uint8_t current = (pips >> (value*9 + 3*(row/2)));
            if (current & (1<<0))
                memset(dest+(2*1), ' ', 2);
            if (current & (1<<1))
                memset(dest+(2*3), ' ', 2);
            if (current & (1<<2))
                memset(dest+(2*5), ' ', 2);
        }

    default:
        break;
    }
};


int main(int argc, char* argv[]) {
    unsigned int count = 1;
    if (argc > 1)
        count = atoi(argv[1]);

    if (count > 10)
        return 1;

    // a single byte contains enough entropy for 42 dice
    uint64_t dice_vals;
    {
        int rand = open("/dev/random", O_RDONLY);
        read(rand, &dice_vals, sizeof(dice_vals));
        close(rand);
    }

    // prepare buffer
    const size_t row_len = dice_row_len*count;
    char* buf = malloc(row_len*7);
    memset(buf, ' ', row_len*7);

    // make it rows
    uint8_t row = 7;
    while (row-- > 0)
        *(buf + ((row+1)*row_len - 1)) = '\n';

    // prepare
    for (unsigned int dice_num = 0; dice_num < count; ++dice_num) {
        uint8_t value = dice_vals % 6 + 1;
        dice_vals /= 6;

        uint8_t row = 7;
        while (row-- > 0)
            set_row_pixels(buf + row_len*row + dice_row_len*dice_num, row, value);
    }

    write(1, buf, row_len*7);

    return 0;
}

