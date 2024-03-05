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

#include "ManagedBuffer.h"
#include <limits.h>
#include "CodalCompat.h"

#define REF_TAG REF_TAG_BUFFER
#define EMPTY_DATA ((BufferData*)(void*)emptyData)

REF_COUNTED_DEF_EMPTY(0, 0)


using namespace std;
using namespace codal;

/**
  * Internal constructor helper.
  * Configures this ManagedBuffer to refer to the static empty buffer.
  */
void ManagedBuffer::initEmpty()
{
    ptr = EMPTY_DATA;
}

/**
 * Default Constructor.
 * Creates an empty ManagedBuffer.
 *
 * Example:
 * @code
 * ManagedBuffer p();
 * @endcode
 */
ManagedBuffer::ManagedBuffer()
{
    initEmpty();
}

/**
 * Constructor.
 * Creates an empty ManagedBuffer of the given size.
 *
 * @param length The length of the buffer to create.
 *
 * Example:
 * @code
 * ManagedBuffer p(16);         // Creates a ManagedBuffer 16 bytes long.
 * @endcode
 */
ManagedBuffer::ManagedBuffer(int length)
{
    this->init(NULL, length);
}

/**
 * Constructor.
 * Creates a new ManagedBuffer of the given size,
 * and fills it with the data provided.
 *
 * @param data The data with which to fill the buffer.
 * @param length The length of the buffer to create.
 *
 * Example:
 * @code
 * uint8_t buf = {13,5,2};
 * ManagedBuffer p(buf, 3);         // Creates a ManagedBuffer 3 bytes long.
 * @endcode
 */
ManagedBuffer::ManagedBuffer(uint8_t *data, int length)
{
    this->init(data, length);
}

/**
 * Copy Constructor.
 * Add ourselves as a reference to an existing ManagedBuffer.
 *
 * @param buffer The ManagedBuffer to reference.
 *
 * Example:
 * @code
 * ManagedBuffer p();
 * ManagedBuffer p2(i);        // Refers to the same buffer as p.
 * @endcode
 */
ManagedBuffer::ManagedBuffer(const ManagedBuffer &buffer)
{
    ptr = buffer.ptr;
    ptr->incr();
}

/**
  * Constructor.
  * Create a buffer from a raw BufferData pointer. It will ptr->incr(). This is to be used by specialized runtimes.
  *
  * @param p The pointer to use.
  */
ManagedBuffer::ManagedBuffer(BufferData *p)
{
    ptr = p;
    ptr->incr();
}

/**
 * Internal constructor-initialiser.
 *
 * @param data The data with which to fill the buffer.
 * @param length The length of the buffer to create.
 *
 */
void ManagedBuffer::init(uint8_t *data, int length)
{
    if (length <= 0) {
        initEmpty();
        return;
    }

    ptr = (BufferData *) malloc(sizeof(BufferData) + length);
    REF_COUNTED_INIT(ptr);

    ptr->length = length;

    // Copy in the data buffer, if provided.
    if (data)
        memcpy(ptr->payload, data, length);
    else
        memset(ptr->payload, 0, length);
}

/**
 * Destructor.
 * Removes buffer resources held by the instance.
 */
ManagedBuffer::~ManagedBuffer()
{
    ptr->decr();
}

/**
 * Copy assign operation.
 *
 * Called when one ManagedBuffer is assigned the value of another using the '=' operator.
 * Decrements our reference count and free up the buffer as necessary.
 * Then, update our buffer to refer to that of the supplied ManagedBuffer,
 * and increase its reference count.
 *
 * @param p The ManagedBuffer to reference.
 *
 * Example:
 * @code
 * uint8_t buf = {13,5,2};
 * ManagedBuffer p1(16);
 * ManagedBuffer p2(buf, 3);
 *
 * p1 = p2;
 * @endcode
 */
ManagedBuffer& ManagedBuffer::operator = (const ManagedBuffer &p)
{
    if(ptr == p.ptr)
        return *this;

    ptr->decr();
    ptr = p.ptr;
    ptr->incr();

    return *this;
}

/**
 * Equality operation.
 *
 * Called when one ManagedBuffer is tested to be equal to another using the '==' operator.
 *
 * @param p The ManagedBuffer to test ourselves against.
 * @return true if this ManagedBuffer is identical to the one supplied, false otherwise.
 *
 * Example:
 * @code
 *
 * uint8_t buf = {13,5,2};
 * ManagedBuffer p1(16);
 * ManagedBuffer p2(buf, 3);
 *
 * if(p1 == p2)                    // will be true
 *     uBit.display.scroll("same!");
 * @endcode
 */
bool ManagedBuffer::operator== (const ManagedBuffer& p)
{
    if (ptr == p.ptr)
        return true;
    else
        return (ptr->length == p.ptr->length && (memcmp(ptr->payload, p.ptr->payload, ptr->length)==0));
}

/**
 * Sets the byte at the given index to value provided.
 * @param position The index of the byte to change.
 * @param value The new value of the byte (0-255).
 * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
 *
 * Example:
 * @code
 * ManagedBuffer p1(16);
 * p1.setByte(0,255);              // Sets the firts byte in the buffer to the value 255.
 * @endcode
 */
int ManagedBuffer::setByte(int position, uint8_t value)
{
    if (0 <= position && (uint16_t)position < ptr->length)
    {
        ptr->payload[position] = value;
        return DEVICE_OK;
    }
    else
    {
        return DEVICE_INVALID_PARAMETER;
    }
}

/**
 * Determines the value of the given byte in the buffer.
 *
 * @param position The index of the byte to read.
 * @return The value of the byte at the given position, or DEVICE_INVALID_PARAMETER.
 *
 * Example:
 * @code
 * ManagedBuffer p1(16);
 * p1.setByte(0,255);              // Sets the firts byte in the buffer to the value 255.
 * p1.getByte(0);                  // Returns 255.
 * @endcode
 */
int ManagedBuffer::getByte(int position)
{
    if (0 <= position && (uint16_t)position < ptr->length)
        return ptr->payload[position];
    else
        return DEVICE_INVALID_PARAMETER;
}

/**
  * Get current ptr, do not decr() it, and set the current instance to an empty buffer.
  * This is to be used by specialized runtimes which pass BufferData around.
  */
BufferData *ManagedBuffer::leakData()
{
    BufferData* res = ptr;
    initEmpty();
    return res;
}


int ManagedBuffer::fill(uint8_t value, int offset, int length)
{
    if (offset < 0 || (uint16_t)offset > ptr->length)
        return DEVICE_INVALID_PARAMETER;
    if (length < 0)
        length = (int)ptr->length;
    length = min(length, (int)ptr->length - offset);

    memset(ptr->payload + offset, value, length);

    return DEVICE_OK;
}

ManagedBuffer ManagedBuffer::slice(int offset, int length) const
{
    offset = min((int)ptr->length, offset);
    if (length < 0)
        length = (int)ptr->length;
    length = min(length, (int)ptr->length - offset);
    return ManagedBuffer(ptr->payload + offset, length);
}

void ManagedBuffer::shift(int offset, int start, int len)
{
    if (len < 0) len = (int)ptr->length - start;
    if (start < 0 || start + len > (int)ptr->length || start + len < start
        || len == 0 || offset == 0 || offset == INT_MIN) return;
    if (offset <= -len || offset >= len) {
        fill(0);
        return;
    }

    uint8_t *data = ptr->payload + start;
    if (offset < 0) {
        offset = -offset;
        memmove(data + offset, data, len - offset);
        memset(data, 0, offset);
    } else {
        len = len - offset;
        memmove(data, data + offset, len);
        memset(data + len, 0, offset);
    }
}

void ManagedBuffer::rotate(int offset, int start, int len)
{
    if (len < 0) len = (int)ptr->length - start;
    if (start < 0 || start + len > (int)ptr-> length || start + len < start
        || len == 0 || offset == 0 || offset == INT_MIN) return;

    if (offset < 0)
        offset += len << 8; // try to make it positive
    offset %= len;
    if (offset < 0)
        offset += len;

    uint8_t *data = ptr->payload + start;

    uint8_t *n_first = data + offset;
    uint8_t *first = data;
    uint8_t *next = n_first;
    uint8_t *last = data + len;

    while (first != next) {
        uint8_t tmp = *first;
        *first++ = *next;
        *next++ = tmp;
        if (next == last) {
            next = n_first;
        } else if (first == n_first) {
            n_first = next;
        }
    }
}

int ManagedBuffer::writeBuffer(int dstOffset, const ManagedBuffer &src, int srcOffset, int length)
{
    if (length < 0)
        length = src.length();

    if (srcOffset < 0 || dstOffset < 0 || dstOffset > (int)ptr->length)
        return DEVICE_INVALID_PARAMETER;

    length = min(src.length() - srcOffset, (int)ptr->length - dstOffset);

    if (length < 0)
        return DEVICE_INVALID_PARAMETER;

    if (ptr == src.ptr) {
        memmove(getBytes() + dstOffset, src.ptr->payload + srcOffset, length);
    } else {
        memcpy(getBytes() + dstOffset, src.ptr->payload + srcOffset, length);
    }

    return DEVICE_OK;
}

int ManagedBuffer::writeBytes(int offset, uint8_t *src, int length, bool swapBytes)
{
    if (offset < 0 || length < 0 || offset + length > (int)ptr->length)
        return DEVICE_INVALID_PARAMETER;

    if (swapBytes) {
        uint8_t *p = ptr->payload + offset + length;
        for (int i = 0; i < length; ++i)
            *--p = src[i];
    } else {
        memcpy(ptr->payload + offset, src, length);
    }

    return DEVICE_OK;
}

int ManagedBuffer::readBytes(uint8_t *dst, int offset, int length, bool swapBytes) const
{
    if (offset < 0 || length < 0 || offset + length > (int)ptr->length)
        return DEVICE_INVALID_PARAMETER;

    if (swapBytes) {
        uint8_t *p = ptr->payload + offset + length;
        for (int i = 0; i < length; ++i)
            dst[i] = *--p;
    } else {
        memcpy(dst, ptr->payload + offset, length);
    }

    return DEVICE_OK;
}

int ManagedBuffer::truncate(int length)
{
    if (length < 0 || length > (int)ptr->length)
        return DEVICE_INVALID_PARAMETER;

    ptr->length = length;

    return DEVICE_OK;
}
