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

#include "WS2812B.h"

using namespace codal;

/**
 * Constructor.
 *
 * @param pwm The PWM peripheral to use to generate the output
 * @param pin The pin connected to the WS2812B pixel strip
 */
WS2812B::WS2812B()
{
    this->downstream = NULL;
    this->setBufferSize(WS2812B_BUFFER_SIZE);
    lock.wait();
} 

/*
    * Allow out downstream component to register itself with us
    */
void WS2812B::connect(DataSink &sink)
{
    this->downstream = &sink;
}


/**
 *  Determine the data format of the buffers streamed out of this component.
 */
int
WS2812B::getFormat()
{
    return DATASTREAM_FORMAT_8BIT_UNSIGNED;
}

/**
 *  Determine the maximum size of the buffers streamed out of this component.
 *  @return The maximum size of this component's output buffers, in bytes.
 */
int
WS2812B::getBufferSize()
{
    return outputBufferSize;
}

/**
 *  Defines the maximum size of the buffers streamed out of this component.
 *  @param size the size of this component's output buffers, in bytes.
 */
int
WS2812B::setBufferSize(int size)
{
    outputBufferSize = size;
    return DEVICE_OK;
}

/**
 * Provide the next available ManagedBuffer to our downstream caller, if available.
 */
ManagedBuffer WS2812B::pull()
{
    // Calculate the amount of data we can transfer in this pull request.
    // Ensure we send at least two buffers, as most downstream components will likely be double buffered...
    int totalSamples = max(outputBufferSize, samplesToSend + 2 * WS2812B_ZERO_PADDING);
    ManagedBuffer buffer(outputBufferSize);

    uint16_t *out = (uint16_t *) &buffer[0];
    uint16_t *end = (uint16_t *) &buffer[buffer.length()];

    while (out < end)
    {
        // Add the front/rear padding if aplicable
        if (samplesSent < WS2812B_ZERO_PADDING || samplesSent > (WS2812B_ZERO_PADDING + samplesToSend))
        {
            *out = WS2812B_PAD;
        }
        else
        {
            int index = (samplesSent - WS2812B_ZERO_PADDING) / 8;
            int bit = (samplesSent - WS2812B_ZERO_PADDING) % 8;

            *out = ((data[index] >> (7-bit)) & 1) ? WS2812B_HIGH : WS2812B_LOW;
        }

        out++;
        samplesSent++;
    }

    // If we still have data to send, indicate this to our downstream component
    if (samplesSent < totalSamples)
        downstream->pullRequest();
    
    // If we have completed playback and blockingbehaviour was requested, wake the fiber that is blocked waiting.
    if ((samplesSent >= totalSamples) && blockingPlayout)
        lock.notify();

    return buffer;
}

/**
 * Perform a non-blocking playout of the given 24 bit RGB/GRB encoded datastream. Thhis method performs no
 * ordering of red/green/blue elements - it simply clocks out the data in the order provided.
 * @param data pointer to the RGB buffer to playout
 * @param length number of bytes in the buffer
 */
void WS2812B::playAsync(const void *data, int length)
{
    _play(data, length, false);
} 

/**
 * Perform a non-blocking playout of the given 24 bit RGB/GRB encoded datastream. Thhis method performs no
 * ordering of red/green/blue elements - it simply clocks out the data in the order provided.
 * @param b the RGB buffer to playout
 */
void WS2812B::playAsync(ManagedBuffer b)
{
    this->play(&b[0], b.length());
}

/**
 * Perform a blocking playout of the given 24 bit RGB/GRB encoded datastream. Thhis method performs no
 * ordering of red/green/blue elements - it simply clocks out the data in the order provided.
 * @param data pointer to the RGB buffer to playout
 * @param length number of bytes in the buffer
 */
void WS2812B::play(const void *data, int length)
{
    _play(data, length, true);
} 

/**
 * Perform a blocking playout of the given 24 bit RGB/GRB encoded datastream. Thhis method performs no
 * ordering of red/green/blue elements - it simply clocks out the data in the order provided.
 * @param b the RGB buffer to playout
 */
void WS2812B::play(ManagedBuffer b)
{
    this->play(&b[0], b.length());
}

void WS2812B::_play(const void *data, int length, bool mode)
{
    if (downstream == NULL || length <= 0)
        return;

    this->data = (uint8_t *)data;
    this->samplesToSend = length * 8;
    this->samplesSent = 0;
    this->blockingPlayout = mode;

    downstream->pullRequest();

    if (this->blockingPlayout)
        lock.wait();
}