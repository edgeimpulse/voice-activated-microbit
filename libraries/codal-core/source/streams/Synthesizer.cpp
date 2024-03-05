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

#include "Synthesizer.h"
#include "CodalFiber.h"
#include "ErrorNo.h"

using namespace codal;

#if 0
// based on ISO 226:2003 - maps frequency to amplitude multiplier
static const uint16_t loudnessCompensation[] = {
    250,  5621, //
    315,  4063, //
    400,  3012, //
    500,  2412, //
    630,  1981, //
    800,  1709, //
    1000, 1699, //
    1250, 2093, //
    1600, 2266, //
    2000, 1553, //
    2500, 1136, //
    3150, 1024, //
    4000, 1154, //
    5000, 1699, //
    6300, 3321, //
    8000, 6604, //
    0, 0
};
#endif

static const uint16_t sineTone[] = {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,3,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,11,11,12,13,13,14,15,16,16,17,18,19,20,21,22,22,23,24,25,26,27,28,29,30,32,33,34,35,36,37,38,40,41,42,43,45,46,47,49,50,51,53,54,56,57,58,60,61,63,64,66,68,69,71,72,74,76,77,79,81,82,84,86,87,89,91,93,95,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,141,143,145,147,149,152,154,156,158,161,163,165,167,170,172,175,177,179,182,184,187,189,191,194,196,199,201,204,206,209,211,214,216,219,222,224,227,229,232,235,237,240,243,245,248,251,253,256,259,262,264,267,270,273,275,278,281,284,287,289,292,295,298,301,304,307,309,312,315,318,321,324,327,330,333,336,339,342,345,348,351,354,357,360,363,366,369,372,375,378,381,384,387,390,393,396,399,402,405,408,411,414,417,420,424,427,430,433,436,439,442,445,448,452,455,458,461,464,467,470,473,477,480,483,486,489,492,495,498,502,505,508,511,514,517,520,524,527,530,533,536,539,542,545,549,552,555,558,561,564,567,570,574,577,580,583,586,589,592,595,598,602,605,608,611,614,617,620,623,626,629,632,635,638,641,644,647,650,653,656,659,662,665,668,671,674,677,680,683,686,689,692,695,698,701,704,707,710,713,715,718,721,724,727,730,733,735,738,741,744,747,749,752,755,758,760,763,766,769,771,774,777,779,782,785,787,790,793,795,798,800,803,806,808,811,813,816,818,821,823,826,828,831,833,835,838,840,843,845,847,850,852,855,857,859,861,864,866,868,870,873,875,877,879,881,884,886,888,890,892,894,896,898,900,902,904,906,908,910,912,914,916,918,920,922,924,926,927,929,931,933,935,936,938,940,941,943,945,946,948,950,951,953,954,956,958,959,961,962,964,965,966,968,969,971,972,973,975,976,977,979,980,981,982,984,985,986,987,988,989,990,992,993,994,995,996,997,998,999,1000,1000,1001,1002,1003,1004,1005,1006,1006,1007,1008,1009,1009,1010,1011,1011,1012,1013,1013,1014,1014,1015,1015,1016,1016,1017,1017,1018,1018,1019,1019,1019,1020,1020,1020,1021,1021,1021,1021,1022,1022,1022,1022,1022,1022,1022,1022,1022,1022,1023,1022};

uint16_t Synthesizer::SineTone(void *arg, int position) {
    int off = TONE_WIDTH - position;
    if (off < TONE_WIDTH / 2)
        position = off;
    return sineTone[position];
}

uint16_t Synthesizer::SawtoothTone(void *arg, int position) {
    return position;
}

uint16_t Synthesizer::TriangleTone(void *arg, int position) {
    return position < 512 ? position * 2 : (1023 - position) * 2;
}

uint16_t Synthesizer::NoiseTone(void *arg, int position) {
    // deterministic, semi-random noise
    uint32_t mult = (uint32_t)arg;
    if (mult == 0)
        mult = 7919;
    return (position * mult) & 1023;
}

uint16_t Synthesizer::SquareWaveTone(void *arg, int position) {
    return position < 512 ? 1023 : 0;
}

uint16_t Synthesizer::SquareWaveToneExt(void *arg, int position) {
    uint32_t duty = (uint32_t)arg;
    return (uint32_t)position <= duty ? 1023 : 0;
}

uint16_t Synthesizer::CustomTone(void *arg, int position) {
    if (position < 0 || position >= 1024)
        return 0;
    return ((uint16_t*)arg)[position];
}

/*
 * Simple internal helper funtion that creates a fiber within the givien Synthesizer to handle playback
 */
static void begin_playback(void *data)
{
    ((Synthesizer*)data)->generate(-1);
}

/**
  * Class definition for a Synthesizer.
  * A Synthesizer generates a tone waveform based on a number of overlapping waveforms.
  */
Synthesizer::Synthesizer(int sampleRate, bool isSigned) : output(*this)
{
    this->isSigned = isSigned;
    this->bufferSize = 512;
    this->samplePeriodNs = 1000000000 / sampleRate;
    this->setVolume(1024);
    this->active = false;
    this->synchronous = false;
    this->bytesWritten = 0;
    this->setTone(Synthesizer::TriangleTone);
    this->position = 0;
    this->status |= DEVICE_COMPONENT_STATUS_IDLE_TICK;
}

/**
 * Implement this function to receive a callback when the device is idling.
 */
void Synthesizer::idleCallback()
{
    if (bytesWritten && !synchronous && !active && output.canPull(bytesWritten))
    {
        buffer.truncate(bytesWritten);
        output.pullRequest();
        bytesWritten = 0;
    }
}


/**
  * Define the central frequency of this synthesizer
  */
int Synthesizer::setFrequency(float frequency)
{
    return setFrequency(frequency, 0);
}

/**
 * Define the central frequency of this synthesizer.
 * Takes effect immediately, and automatically stops the tone after the given period.
 * @frequency The frequency, in Hz to generate.
 * @period The period, in ms, to play the frequency.
 */
int Synthesizer::setFrequency(float frequency, int period, int envelopeStart, int envelopeEnd)
{
    // If another fiber is already actively using this resource, we can't service this request.
    if (synchronous)
        return DEVICE_BUSY;

    // record our new intended frequency.
    newPeriodNs = frequency == 0.0 ? 0 : (uint32_t) (1000000000.0f / frequency);

    if (period == 0)
    {
        // We've been asked to play a new tone in the background.
        // If a tone is already playing in the background, we only need to update frequency (already done above). Otherwise also launch a playout fiber.
        if(!active)
        {
            active = true;
            create_fiber(begin_playback, this);
        }
    }
    else
    {
        // We've been asked to playout a new note synchronously. Record the period of playback, and start creation of the sample content.
        synchronous = true;
        generate(1000 * period, envelopeStart, envelopeEnd);
        synchronous = false;
    }

    return DEVICE_OK;
}

/**
 * Destructor.
 * Removes all resources held by the instance.
 */
Synthesizer::~Synthesizer()
{
}

/**
 * Creates the next audio buffer, and attmepts to queue this on the output stream.
 */
void Synthesizer::generate(int playoutTimeUs, int envelopeStart, int envelopeEnd)
{
    int periodNs = newPeriodNs;

    int toneDelta;          // the number of samples within our tone print that we increment for each playout sample.
    int toneSigma;          // the fraction of samples within our tone print (*1000) that we increment for each playout sample.

    float toneRate = periodNs == 0 ? 0 : ((float)samplePeriodNs * (float) TONE_WIDTH) / (float) periodNs;
    toneDelta = (int) toneRate;
    toneSigma = (int) ((toneRate - (float)toneDelta) * 1000.0f);

    int sigma = 0;
    int playoutSamples = determineSampleCount(playoutTimeUs);

    int localAmplitude = (amplitude * envelopeStart) << 10;
    int localAmplitudeDelta = ((envelopeEnd - envelopeStart) << 20) / playoutSamples;

    while(playoutSamples != 0)
    {
        if (bytesWritten == 0)
            buffer = ManagedBuffer(bufferSize);

        uint16_t *ptr = (uint16_t *) &buffer[bytesWritten];

        if (playoutTimeUs < 0)
            localAmplitude = amplitude << 20;
        else
            localAmplitude += localAmplitudeDelta;

        while(bytesWritten < bufferSize)
        {
            if (periodNs <= 0)
                *ptr = 0;
            else if (isSigned)
                *ptr = (((int)tonePrint(tonePrintArg, position) - 512) * (localAmplitude >> 20)) >> 10;
            else
                *ptr = (tonePrint(tonePrintArg, position) * (localAmplitude >> 20)) >> 10;
            bytesWritten += 2;
            ptr++;

            position += toneDelta;
            sigma += toneSigma;

            if (playoutSamples >= 0)
                playoutSamples--;

            if (sigma > 1000)
            {
                sigma -= 1000;
                position++;
            }

            while (position >= TONE_WIDTH) // most likely at most one iteration
            {
                position -= TONE_WIDTH;
#ifdef SYNTHESIZER_SIGMA_RESET
                sigma = 0;
#endif
                if (periodNs != newPeriodNs)
                {
                    periodNs = newPeriodNs;

                    toneRate = periodNs == 0 ? 0 : ((float)samplePeriodNs * (float) TONE_WIDTH) / (float) periodNs;
                    toneDelta = (int) toneRate;
                    toneSigma = (int) ((toneRate - (float)toneDelta) * 1000.0f);
                    playoutSamples = determineSampleCount(playoutTimeUs);

                    position = 0;
                    sigma = 0;
                }
            }

            if (playoutSamples == 0)
                return;
        }

        bytesWritten = 0;
        output.pullRequest();

        // There's now space for another buffer. If we're generating asynchronously and a synchronous request comes in, give control to that fiber.
        if (playoutTimeUs < 0 && synchronous)
        {
            active = false;
            return;
        }
    }
}

/**
* Define the volume of the wave to generate.
* @param volume The new output volume, in the range 0..1024
*/
int Synthesizer::setVolume(int volume)
{
    if (volume < 0 || volume > 1024)
        return DEVICE_INVALID_PARAMETER;

    amplitude = volume;

    return DEVICE_OK;
}

/**
* Define the size of the audio buffer to hold. The larger the buffer, the lower the CPU overhead, but the longer the delay.
* @param size The new bufer size to use, in bytes.
* @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER
*/
int Synthesizer::setBufferSize(int size)
{
    if (bufferSize <= 0)
        return DEVICE_INVALID_PARAMETER;

    this->bufferSize = size;
    return DEVICE_OK;
}

/**
 * Provide the next available ManagedBuffer to our downstream caller, if available.
 */
ManagedBuffer Synthesizer::pull()
{
    ManagedBuffer out = buffer;
    buffer = ManagedBuffer();
    return out;
}

/**
 * Determine the sample rate currently in use by this Synthesizer.
 * @return the current sample rate, in Hz.
 */
int Synthesizer::getSampleRate()
{
    return 1000000000 / samplePeriodNs;
}

/**
 * Change the sample rate used by this Synthesizer,
 * @param frequency The new sample rate, in Hz.
 * @return DEVICE_OK on success.
 */
int Synthesizer::setSampleRate(int sampleRate)
{
    this->samplePeriodNs = 1000000000 / sampleRate;
    return DEVICE_OK;
}

/**
 * Defines the tone to generate.
 * @param the tone print to use with this synthesizer.
 * Examples include:
 *
 * Synthesizer::SineTone
 * Synthesizer::SawtoothTone
 * Synthesizer::SquareWaveTone
 */
void Synthesizer::setTone(SynthesizerGetSample tonePrint, void *arg)
{
    this->tonePrintArg = arg;
    this->tonePrint = tonePrint;
}

/**
 * Determine the number of samples required for the given playout time.
 *
 * @param playoutTimeUs The playout time (in microseconds)
 * @return The number if samples required to play for the given amount fo time
 * (at the currently defined sample rate)
 */
int Synthesizer::determineSampleCount(int playoutTimeUs)
{
    if (playoutTimeUs < 0)
        return -1;

    int a = (playoutTimeUs / 1000) * 1000;
    int b = (playoutTimeUs % 1000);

    return ((a / samplePeriodNs) * 1000) + ((1000 * b) / samplePeriodNs);
}

