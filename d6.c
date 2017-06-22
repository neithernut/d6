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
#include <stdint.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/uio.h>
#include <unistd.h>


/**
 * This program prints dice faces for random values as text. Each pixel is made
 * up of two characters. A dice face is printed using 7x7 pixels, the dice are
 * separated by one pixel.
 */


/**
 * Dice data
 *
 * There are nine positions for pips on each d6, which we enumerate starting
 * form `0`:
 *
 *     #######
 *     #0 1 2#
 *     #3 4 5#
 *     #6 7 8#
 *     #######
 *
 * Hence, we can represent the pip-configuration of a dice using 9 bits. The
 * six configurations for a d6 easily fit into a 64bit integer value. We even
 * have the luxury of starting at an offset without wasting memory.
 */
const uint64_t pips =
    ((uint64_t) 0020) << (1*9) |
    ((uint64_t) 0104) << (2*9) |
    ((uint64_t) 0124) << (3*9) |
    ((uint64_t) 0505) << (4*9) |
    ((uint64_t) 0525) << (5*9) |
    ((uint64_t) 0555) << (6*9);


/**
 * The length of a row for one dice, including the space after the dice
 *
 * 7 pixels for the dice face + the separating pixel, times 2 characters per
 * pixel.
 */
const size_t dice_row_len = 16;


/**
 * Variations of lines which occur in a dice
 *
 * A line contains up to three pips, each of which may be represented by one
 * bit. Hence, eight differente variations of a line exist, enumerable via the
 * three bits. However, only a subset of those lines do occur.
 */
char const* dice_parts[] = {
    "##############  ",
    "##  ##########  ",
    "######  ######  ",
    NULL,
    "##########  ##  ",
    "##  ######  ##  "
};


/**
 * Get an iovec for a horizontal line of pixels/characters of a dice face
 */
struct iovec
row_vec(
    uint8_t row, ///< row of the dice face to write
    uint8_t value ///< value shown by the dice face
) {
    struct iovec retval;

    uint8_t part = 0;
    if (row & 1)
        part = (pips >> (value*9 + 3*(row/2))) & 7;

    retval.iov_base = (void*) dice_parts[part];
    retval.iov_len = dice_row_len;
    return retval;
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

    // prepare iovec
    size_t const vec_count = 7 * (count + 1);
    struct iovec* vecs = alloca(sizeof(struct iovec) * vec_count);

    // put line ends
    {
        uint8_t row = 7;
        while (row-- > 0) {
            struct iovec* line_end = vecs + (row * (count+1) + count);
            line_end->iov_base = "\n";
            line_end->iov_len = 1;
        }
    }

    // prepare
    for (unsigned int dice_num = 0; dice_num < count; ++dice_num) {
        uint8_t value = dice_vals % 6 + 1;
        dice_vals /= 6;

        uint8_t row = 7;
        while (row-- > 0)
            vecs[row * (count+1) + dice_num] = row_vec(row, value);
    }

    writev(1, vecs, vec_count);

    return 0;
}

