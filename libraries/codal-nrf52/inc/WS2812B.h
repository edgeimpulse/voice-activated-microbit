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

#include "CodalConfig.h"
#include "DataStream.h"
#include "NRF52PWM.h"
#include "NRF52Pin.h"

#ifndef WS2812B_H
#define WS2812B_H

#define WS2812B_BUFFER_SIZE         256
#define WS2812B_PAD                 (0x8000)
#define WS2812B_LOW                 (0x8000 | 6)
#define WS2812B_HIGH                (0x8000 | 10)
#define WS2812B_PWM_FREQ            500000
#define WS2812B_ZERO_PADDING        50

/**
 * A simple buffer class for encoding an streaming WS2812B (neopixel) data via a NRF52PWM peripheral
 */
namespace codal
{
    class WS2812B : public DataSource
    {
        private:

        int             outputBufferSize;       // The maximum size of an output buffer.

        uint8_t         *data;                  // The input data being played (immutable)
        int             samplesToSend;          // The length of the input buffer (mutable)
        int             samplesSent;            // The number of bytes sent so far in the current request

        DataSink        *downstream;            // Pointer to our downstream component
        bool            blockingPlayout;        // Set to true if a blocking playout has been requested
        FiberLock       lock;                   // used to synchronise blocking play calls.

        public:

        /**
         * Constructor.
         *
         * @param pwm The PWM peripheral to use to generate the output
         * @param pin The pin connected to the WS2812B pixel strip
         */
        WS2812B();

        /**
         * Provide the next available ManagedBuffer to our downstream caller, if available.
         */
        virtual ManagedBuffer pull();

        /**
         *  Determine the data format of the buffers streamed out of this component.
         */
        virtual int getFormat();

        /*
         * Allow out downstream component to register itself with us
         */
        virtual void connect(DataSink &sink);

        /**
         *  Determine the maximum size of the buffers streamed out of this component.
         *  @return The maximum size of this component's output buffers, in bytes.
         */
        int getBufferSize();

        /**
         *  Defines the maximum size of the buffers streamed out of this component.
         *  @param size the size of this component's output buffers, in bytes.
         */
        int setBufferSize(int size);

        /**
         * Perform a blocking playout of the given 24 bit RGB/GRB encoded datastream. Thhis method performs no
         * ordering of red/green/blue elements - it simply clocks out the data in the order provided.
         * @param data pointer to the RGB buffer to playout
         * @param length number of bytes in the buffer
         */
        void play(const void *data, int length);

        /**
         * Perform a blocking playout of the given 24 bit RGB/GRB encoded datastream. Thhis method performs no
         * ordering of red/green/blue elements - it simply clocks out the data in the order provided.
         * @param b the RGB buffer to playout
         */
        void play(ManagedBuffer b);

        /**
         * Perform a non-blocking playout of the given 24 bit RGB/GRB encoded datastream. Thhis method performs no
         * ordering of red/green/blue elements - it simply clocks out the data in the order provided.
         * @param data pointer to the RGB buffer to playout
         * @param length number of bytes in the buffer
         */
        void playAsync(const void *data, int length);

        /**
         * Perform a non-blocking playout of the given 24 bit RGB/GRB encoded datastream. Thhis method performs no
         * ordering of red/green/blue elements - it simply clocks out the data in the order provided.
         * @param b the RGB buffer to playout
         */
        void playAsync(ManagedBuffer b);


        private:
        void _play(const void *data, int length, bool mode);
    };
}
#endif