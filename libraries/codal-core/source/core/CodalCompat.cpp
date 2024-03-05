/*
The MIT License (MIT)

Copyright (c) 2017 Lancaster University.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

/**
  * This file contains functions used to maintain compatability and portability.
  * It also contains constants that are used elsewhere in the DAL.
  */
#include "CodalConfig.h"
#include "CodalCompat.h"
#include "ErrorNo.h"

static uint32_t random_value;

/**
  * Performs an in buffer reverse of a given char array.
  *
  * @param s the string to reverse.
  *
  * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
  */
int codal::string_reverse(char *s)
{
    //sanity check...
    if(s == NULL)
        return DEVICE_INVALID_PARAMETER;

    char *j;
    int c;

    j = s + strlen(s) - 1;

    while(s < j)
    {
        c = *s;
        *s++ = *j;
        *j-- = c;
    }

    return DEVICE_OK;
}

/**
  * Converts a given integer into a string representation.
  *
  * @param n The number to convert.
  *
  * @param s A pointer to the buffer where the resulting string will be stored.
  *
  * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
  */
int codal::itoa(int n, char *s)
{
    int i = 0;
    int positive = (n >= 0);

    if (s == NULL)
        return DEVICE_INVALID_PARAMETER;

    // Record the sign of the number,
    // Ensure our working value is positive.
    if (positive)
        n = -n;

    // Calculate each character, starting with the LSB.
    do {
         s[i++] = abs(n % 10) + '0';
    } while (abs(n /= 10) > 0);

    // Add a negative sign as needed
    if (!positive)
        s[i++] = '-';

    // Terminate the string.
    s[i] = '\0';

    // Flip the order.
    string_reverse(s);

    return DEVICE_OK;
}

int codal::seed_random(uint32_t seed)
{
    random_value = seed;
    return DEVICE_OK;
}

int codal::random(int max)
{
    uint32_t m, result;

    if (max <= 0)
        return DEVICE_INVALID_PARAMETER;

    if (random_value == 0)
        seed_random(0xC0DA1);

    // Our maximum return value is actually one less than passed
    max--;

    do
    {
        m = (uint32_t)max;
        result = 0;
        do
        {
            // Cycle the LFSR (Linear Feedback Shift Register).
            // We use an optimal sequence with a period of 2^32-1, as defined by Bruce Schneier here
            // (a true legend in the field!),
            // For those interested, it's documented in his paper:
            // "Pseudo-Random Sequence Generator for 32-Bit CPUs: A fast, machine-independent
            // generator for 32-bit Microprocessors"
            // https://www.schneier.com/paper-pseudorandom-sequence.html
            uint32_t rnd = random_value;

            rnd = ((((rnd >> 31) ^ (rnd >> 6) ^ (rnd >> 4) ^ (rnd >> 2) ^ (rnd >> 1) ^ rnd) &
                    0x0000001)
                   << 31) |
                  (rnd >> 1);

            random_value = rnd;

            result = ((result << 1) | (rnd & 0x00000001));
        } while (m >>= 1);
    } while (result > (uint32_t)max);

    return result;
}