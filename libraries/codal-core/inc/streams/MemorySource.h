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

#include "CodalConfig.h"
#include "DataStream.h"

#ifndef MEMORY_SOURCE_H
#define MEMORY_SOURCE_H

#define MEMORY_SOURCE_DEFAULT_MAX_BUFFER        256

/**
 * A simple buffer class for streaming bytes from memory across the Stream APIs.
 */
namespace codal
{
    class MemorySource : public DataSource
    {
        private:
        int             outputFormat;           // The format to output in. By default, this is the same as the input.
        int             outputBufferSize;       // The maximum size of an output buffer.
        ManagedBuffer   buffer;                 // The output buffer being filled

        uint8_t         *data;                  // The input data being played (immutable)
        uint8_t         *in;                    // The input data being played (mutable)
        int             length;                 // The lenght of the input buffer (immutable)
        int             bytesToSend;            // The lenght of the input buffer (mutable)
        int             count;                   // The number of times left to repeat

        DataSink        *downstream;            // Pointer to our downstream component
        bool            blockingPlayout;        // Set to true if a blocking playout has been requested
        FiberLock       lock;                   // used to synchronise blocking play calls.

        public:
        DataSource      &output;                // DEPRECATED: backward compatilbity only

        /**
         * Default Constructor.
         */
        MemorySource();

        /**
         * Provide the next available ManagedBuffer to our downstream caller, if available.
         */
        virtual ManagedBuffer pull();

        /**
         *  Determine the data format of the buffers streamed out of this component.
         */
        virtual int getFormat();

        /**
         * Defines the data format of the buffers streamed out of this component.
         * @param format valid values include:
         * 
         * DATASTREAM_FORMAT_8BIT_UNSIGNED
         * DATASTREAM_FORMAT_8BIT_SIGNED
         * DATASTREAM_FORMAT_16BIT_UNSIGNED
         * DATASTREAM_FORMAT_16BIT_SIGNED
         * DATASTREAM_FORMAT_24BIT_UNSIGNED
         * DATASTREAM_FORMAT_24BIT_SIGNED
         * DATASTREAM_FORMAT_32BIT_UNSIGNED
         * DATASTREAM_FORMAT_32BIT_SIGNED
         */
        virtual int setFormat(int format);

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
         * Perform a blocking playout of the data buffer. Returns when all the data has been queued.
         * @param data pointer to memory location to playout
         * @param length number of samples in the buffer. Assumes a sample size as defined by setFormat().
         * @param count if set, playback the buffer the given number of times. Defaults to 1. Set to a negative number to loop forever.
         */
        void play(const void *data, int length, int count = 1);

        /**
         * Perform a blocking playout of the data buffer. Returns when all the data has been queued.
         * @param b the buffer to playout
         * @param count if set, playback the buffer the given number of times. Defaults to 1. Set to a negative number to loop forever.
         */
        void play(ManagedBuffer b, int count = 1);

        /**
         * Perform a blocking playout of the data buffer. Returns when all the data has been queued.
         * @param data pointer to memory location to playout
         * @param length number of samples in the buffer. Assumes a sample size as defined by setFormat().
         * @param count if set, playback the buffer the given number of times. Defaults to 1. Set to a negative number to loop forever.
         */
        void playAsync(const void *data, int length, int count = 1);

        /**
         * Perform a blocking playout of the data buffer. Returns when all the data has been queued.
         * @param b the buffer to playout
         * @param count if set, playback the buffer the given number of times. Defaults to 1. Set to a negative number to loop forever.
         */
        void playAsync(ManagedBuffer b, int count = 1);


        private:
        void _play(const void *data, int length, int count, bool mode);
    };
}
#endif