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

#ifndef CODAL_DMESG_H
#define CODAL_DMESG_H

#include "CodalConfig.h"
#include <stdbool.h>

#if DEVICE_DMESG_BUFFER_SIZE > 0

#if DEVICE_DMESG_BUFFER_SIZE < 256
#error "Too small DMESG buffer"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct CodalLogStore
{
    uint32_t ptr;
    char buffer[DEVICE_DMESG_BUFFER_SIZE];
};
extern struct CodalLogStore codalLogStore;

/**
  * Log formatted message to an internal buffer.
  *
  * Supported format strings:
  *    %c - single character
  *    %d - decimal number
  *    %x - hexadecimal number (with 0x)
  *    %p - hexadecimal number padded with zeros (and with 0x)
  *    %X - hexadecimal number padded with zeros (and with 0x)
  *    %s - '\0'-terminated string
  *    %% - literal %
  * Typically used via the DMESG() macro.
  *
  * @param format Format string
  *
  * @code
  * uint32_t k;
  * void *ptr;
  * ...
  * DMESG("USB: Error #%d at %X", k, ptr);
  * @endcode
  */
void codal_dmesg(const char *format, ...);
void codal_dmesg_nocrlf(const char *format, ...);
void codal_dmesg_with_flush(const char *format, ...);

void codal_dmesg_set_flush_fn(void (*fn)(void));
void codal_dmesg_flush();

void codal_vdmesg(const char *format, bool crlf, va_list ap);

#define DMESG   codal_dmesg
#define DMESGN  codal_dmesg_nocrlf
#define DMESGF  codal_dmesg_with_flush

#ifdef __cplusplus
}
#endif

#else

#define DMESG(...) ((void)0)
#define DMESGN(...) ((void)0)
#define DMESGF(...) ((void)0)

#endif

#endif
