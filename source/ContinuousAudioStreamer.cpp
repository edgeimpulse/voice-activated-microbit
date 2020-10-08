/*
The MIT License (MIT)

Copyright (c) 2020 EdgeImpulse Inc.

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

#include "ContinuousAudioStreamer.h"
#include "Tests.h"

/**
 * Creates a simple component that logs a stream of signed 16 bit data as signed 8-bit data over serial.
 * @param source a DataSource to measure the level of.
 * @param inference an initialized inference (with all buffers allocated of inference_t struct)
 */
ContinuousAudioStreamer::ContinuousAudioStreamer(DataSource &source, inference_t *inference) : upstream(source)
{
    this->_inference = inference;

    // Register with our upstream component
    source.connect(*this);
}

/**
 * Callback provided when data is ready.
 */
int ContinuousAudioStreamer::pullRequest()
{
    static volatile int pr = 0;

    if(!pr)
    {
        pr++;
        while(pr)
        {
            lastBuffer = upstream.pull();
            streamBuffer(lastBuffer);
            pr--;
        }
    }
    else
    {
        pr++;
    }

    return DEVICE_OK;
}

/**
 * returns the last buffer processed by this component
 */
ManagedBuffer ContinuousAudioStreamer::getLastBuffer()
{
    return lastBuffer;
}

/**
 * Callback provided when data is ready.
 */
void ContinuousAudioStreamer::streamBuffer(ManagedBuffer buffer)
{
    int CRLF = 0;
    int bps = upstream.getFormat();

    static int irq_counter = 0;
    static int last_print = 0;

    ++irq_counter;

    uint8_t *p = &buffer[0];
    uint8_t *end = p + buffer.length();

    while (p < end) {
        int8_t data = *p++;

        _inference->buffers[_inference->buf_select][_inference->buf_count++] = data;

        if (_inference->buf_count >= _inference->n_samples) {
            if (_inference->buf_ready == 1) {
                // uBit.serial.printf(
                //     "Error sample buffer overrun. Decrease the number of slices per model window "
                //     "(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)\n");
            }

            _inference->buf_select ^= 1;
            _inference->buf_count = 0;
            _inference->buf_ready = 1;

            // uBit.serial.printf("IRQ Counter: %d (buf length = %d), %d\n", irq_counter,
            //     static_cast<int>(buffer.length()), system_timer_current_time() - last_print);
            irq_counter = 0;
            last_print = system_timer_current_time();
        }
    }
}
