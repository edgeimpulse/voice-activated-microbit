/*
The MIT License (MIT)

Copyright (c) 2020 Lancaster University.

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

#include "MemorySource.h"
#include "CodalDmesg.h"

using namespace codal;

/**
 * Default Constructor.
 *
 * @param data pointer to memory location to playout
 * @param length number of bytes to stream
 */
MemorySource::MemorySource() : output(*this)
{
    this->downstream = NULL;
    this->setFormat(DATASTREAM_FORMAT_8BIT_UNSIGNED);
    this->setBufferSize(MEMORY_SOURCE_DEFAULT_MAX_BUFFER);
    lock.wait();
} 

/*
    * Allow out downstream component to register itself with us
    */
void MemorySource::connect(DataSink &sink)
{
    this->downstream = &sink;
}


/**
 *  Determine the data format of the buffers streamed out of this component.
 */
int
MemorySource::getFormat()
{
    return outputFormat;
}

/**
 * Defines the data format of the buffers streamed out of this component.
 * @param format the format to use, one of
 */
int
MemorySource::setFormat(int format)
{
    outputFormat = format;
    return DEVICE_OK;
}

/**
 *  Determine the maximum size of the buffers streamed out of this component.
 *  @return The maximum size of this component's output buffers, in bytes.
 */
int
MemorySource::getBufferSize()
{
    return outputBufferSize;
}

/**
 *  Defines the maximum size of the buffers streamed out of this component.
 *  @param size the size of this component's output buffers, in bytes.
 */
int
MemorySource::setBufferSize(int size)
{
    outputBufferSize = size;
    return DEVICE_OK;
}

/**
 * Provide the next available ManagedBuffer to our downstream caller, if available.
 */
ManagedBuffer MemorySource::pull()
{
    // Calculate the amount of data we can transfer.
    int l = min(bytesToSend, outputBufferSize);
    buffer = ManagedBuffer(l);

    memcpy(&buffer[0], in, l);

    bytesToSend -= l;
    in += l;

    // If we've consumed the input buffer, see if we need to reload it
    if (bytesToSend == 0)
    {
        if (count > 0)
            count--;

        if (count != 0)
        {
            bytesToSend = length;
            in = data;
        }
    }

    // If we still have data to send, indicate this to our downstream component
    if (bytesToSend > 0)
        downstream->pullRequest();
    
    // If we have completed playback and blockingbehaviour was requested, wake the fiber that is blocked waiting.
    if (bytesToSend == 0 && count == 0 && blockingPlayout)
        lock.notify();

    return buffer;
}

/**
 * Perform a non-blocking playout of the data buffer. Returns when all the data has been queued.
 * @param data pointer to memory location to playout
 * @param length number of samples in the buffer. Assumes a sample size as defined by setFormat().
 * @param count if set, playback the buffer the given number of times. Defaults to 1. Set to a negative number to loop forever.
 */
void MemorySource::playAsync(const void *data, int length, int count)
{
    _play(data, length, count, false);
} 

/**
 * Perform a non-blocking playout of the data buffer. Returns when all the data has been queued.
 * @param b the buffer to playout
 * @param count if set, playback the buffer the given number of times. Defaults to 1. Set to a negative number to loop forever.
 */
void MemorySource::playAsync(ManagedBuffer b, int count)
{
    this->play(&b[0], b.length(), count);
}

/**
 * Perform a blocking playout of the data buffer. Returns when all the data has been queued.
 * @param data pointer to memory location to playout
 * @param length number of samples in the buffer. Assumes a sample size as defined by setFormat().
 * @param count if set, playback the buffer the given number of times. Defaults to 1. Set to a negative number to loop forever.
 */
void MemorySource::play(const void *data, int length, int count)
{
    _play(data, length, count, true);
} 

/**
 * Perform a blocking playout of the data buffer. Returns when all the data has been queued.
 * @param b the buffer to playout
 * @param count if set, playback the buffer the given number of times. Defaults to 1. Set to a negative number to loop forever.
 */
void MemorySource::play(ManagedBuffer b, int count)
{
    this->play(&b[0], b.length(), count);
}

void MemorySource::_play(const void *data, int length, int count, bool mode)
{
    if (downstream == NULL || length <= 0 || count == 0)
        return;

    this->data = this->in = (uint8_t *)data;
    this->length =this->bytesToSend = length;
    this->count = count;
    this->blockingPlayout = mode;

    downstream->pullRequest();

    if (this->blockingPlayout)
        lock.wait();
}