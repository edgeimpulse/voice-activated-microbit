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

#ifndef STREAM_NORMALIZER_H
#define STREAM_NORMALIZER_H

/**
 * Sample read/write functions for 8, 16, 24, 32 bit signed/unsigned data.
 */
typedef int (*SampleReadFn)(uint8_t *);
typedef void (*SampleWriteFn)(uint8_t *, int);


/**
 * Default configuration values
 */

namespace codal{

    class StreamNormalizer : public DataSink, public DataSource
    {
    public:
        int             outputFormat;           // The format to output in. By default, this is the sme as the input.
        int             stabilisation;          // The % stability of the zero-offset calculation required to begin operation.
        float           gain;                   // Gain to apply.
        float           zeroOffset;             // Best estimate of the zero point of the data source.
        uint32_t        orMask;                 // post processing step - or'd with each sample.
        bool            normalize;              // If set, will recalculate a zero offset.
        bool            zeroOffsetValid;        // Set to true after the first buffer has been processed.
        bool            outputEnabled;          // When set any buffer processed will be forwarded downstream.
        DataSource      &upstream;              // The upstream component of this StreamNormalizer.
        DataStream      output;                 // The downstream output stream of this StreamNormalizer.
        ManagedBuffer   buffer;                 // The buffer being processed.

        static SampleReadFn readSample[9];
        static SampleWriteFn writeSample[9];

        /**
          * Creates a component capable of translating one data representation format into another
          *
          * @param source a DataSource to receive data from
          * @param gain The gain to apply to each sample (default: 1.0)
          * @param normalize Derive a zero offset for the input stream, and subtract from each sample (default: false)
          * @param format The format to convert the input stream into
          * @param stabilisation the maximum change of zero-offset permitted between subsequent buffers before output is initiated. Set to zero to disable (default)
          */
        StreamNormalizer(DataSource &source, float gain = 1.0f, bool normalize = false, int format = DATASTREAM_FORMAT_UNKNOWN, int stabilisation = 0);

        /**
         * Callback provided when data is ready.
         */
    	virtual int pullRequest();

        /**
         * Provide the next available ManagedBuffer to our downstream caller, if available.
         */
        virtual ManagedBuffer pull();

        /**
         * Defines whether this input stream will be normalized based on its mean average value.
         *
         * @param normalize The state to apply - set to true to apply normlization, false otherwise.
         * @return DEVICE_OK on success.
         */
        int setNormalize(bool normalize);

        /**
         * Determines whether normalization is being applied .
         * @return true if normlization is being performed, false otherwise.
         */
        bool getNormalize();

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

        /**
         * Defines an optional gain to apply to the input, as afloating point multiple.
         *
         * @param gain The gain to apply to this input stream.
         * @return DEVICE_OK on success.
         */
        int setGain(float gain);

        /**
         * Determines the  gain being applied to the input, as a floating point multiple.
         * @return the gain applied.
         */
        float getGain();

        /**
         * Defines an optional bit mask to logical OR with each sample.
         * Useful if the downstream component encodes control data within its samples.
         *
         * @param mask The bitmask to to apply to each sample.
         * @return DEVICE_OK on success.
         */
        int setOrMask(uint32_t mask);

        /**
         * Destructor.
         */
        ~StreamNormalizer();

    };
}

#endif
