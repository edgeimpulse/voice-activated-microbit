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

#ifndef CODAL_SYNTHESIZER_H
#define CODAL_SYNTHESIZER_H

#include "DataStream.h"

#define SYNTHESIZER_SAMPLE_RATE        44100
#define TONE_WIDTH                  1024

namespace codal
{
    typedef uint16_t (*SynthesizerGetSample)(void *arg, int position);

    /**
      * Class definition for DataStream.
      * A Datastream holds a number of ManagedBuffer references, provides basic flow control through a push/pull mechanism
      * and byte level access to the datastream, even if it spans different buffers.
      */
    class Synthesizer : public DataSource, public CodalComponent
    {
        int     samplePeriodNs;        // The length of a single sample, in nanoseconds.
        int     bufferSize;            // The number of samples to create in a single buffer before scheduling it for playback

        int     newPeriodNs;           // new period of waveform, if change has been requested.
        int     amplitude;             // The maximum amplitude of the wave to generate (the volume of the output)
        bool    active;                // Determines if background playback of audio is currently active.
        bool    synchronous;           // Determines if a synchronous mode of operation has been requested.
        bool    isSigned;              // If true, samples use int16_t otherwise uint16_t.

        ManagedBuffer buffer;          // Playout buffer.
        int     bytesWritten;          // Number of bytes written to the output buffer.
        void*   tonePrintArg;
        SynthesizerGetSample tonePrint;     // The tone currently selected playout tone (always unsigned).
        int     position;              // Position within the tonePrint

        public:

        DataStream output;

        static uint16_t SineTone(void *arg, int position);
        static uint16_t SawtoothTone(void *arg, int position);
        static uint16_t TriangleTone(void *arg, int position);
        static uint16_t SquareWaveTone(void *arg, int position);
        static uint16_t SquareWaveToneExt(void *arg, int position);
        static uint16_t NoiseTone(void *arg, int position);
        static uint16_t CustomTone(void *arg, int position);

        /**
          * Default Constructor.
          * Creates an empty DataStream.
          *
          * @param sampleRate The sample rate at which this synthesizer will produce data.
          */
        Synthesizer(int sampleRate = SYNTHESIZER_SAMPLE_RATE, bool isSigned = false);

        /**
          * Destructor.
          * Removes all resources held by the instance.
          */
        ~Synthesizer();

        /**
        * Define the central frequency of this synthesizer. Takes effect at the start of the next waveform period.
        * @param frequency The frequency, in Hz to generate.
        */
        int setFrequency(float frequency);

        /**
        * Define the central frequency of this synthesizer. Takes effect at the start of the next waveform period.
        * @param frequency The frequency, in Hz to generate.
        * @param period The period, in ms, to play the frequency.
        * @param envelopeStart Starting amplitude (multiplied by general volume of this synth)
        * @param envelopeEnd Final amplitude (multiplied by general volume of this synth)
        */
        int setFrequency(float frequency, int period, int envelopeStart = 1024, int envelopeEnd = 1024);

        /**
        * Define the volume of the wave to generate.
        * @param volume The new output volume, in the range 0..1023
        * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER
        */
        int setVolume(int volume);

        /**
        * Define the size of the audio buffer to hold. The larger the buffer, the lower the CPU overhead, but the longer the delay.
        * @param size The new bufer size to use.
        * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER
        */
        int setBufferSize(int size);

        /**
         * Determine the sample rate currently in use by this Synthesizer.
         * @return the current sample rate, in Hz.
         */
        int getSampleRate();

        /**
         * Change the sample rate used by this Synthesizer,
         * @param sampleRate The new sample rate, in Hz.
         * @return DEVICE_OK on success.
         */
        int setSampleRate(int sampleRate);

        /**
         * Provide the next available ManagedBuffer to our downstream caller, if available.
         */
        virtual ManagedBuffer pull();

        /**
         * Implement this function to receive a callback when the device is idling.
         */
        virtual void idleCallback();

        /**
        * Creates the next audio buffer, and attmepts to queue this on the output stream.
        */
        void generate(int playoutTimeUs, int envelopeStart = 1024, int envelopeEnd = 1024);

        /**
         * Defines the tone to generate.
         * @param the tone print to use with this synthesizer.
         * Examples include:
         *
         * Synthesizer::SineTone
         * Synthesizer::SawtoothTone
         * Synthesizer::TriangleTone
         * Synthesizer::SquareWaveTone
         * Synthesizer::SquareWaveToneExt (with argument which is duty cycle 0-1023)
         */
        void setTone(SynthesizerGetSample tonePrint, void *arg = NULL);
        // legacy
        void setTone(const uint16_t *tonePrint) { setTone(CustomTone, (void*)tonePrint); }

        private:

        /**
         * Determine the number of samples required for the given playout time.
         *
         * @param playoutTimeUs The playout time (in microseconds)
         * @return The number if samples required to play for the given amount fo time
         * (at the currently defined sample rate)
         */
        int determineSampleCount(int playoutTimeUs);

    };
}

#endif
