/*
The MIT License (MIT)

Copyright (c) 2016 Lancaster University.

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

#include "SerialStreamer.h"
#include "Tests.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"

static int8_t ss_buffer[EI_CLASSIFIER_RAW_SAMPLE_COUNT];
static size_t ss_buffer_ix = 0;

static int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    return numpy::int8_to_float(ss_buffer + offset, out_ptr, length);
}

const char * const happy_emoji ="\
    000,255,000,255,000\n\
    000,000,000,000,000\n\
    255,000,000,000,255\n\
    000,255,255,255,000\n\
    000,000,000,000,000\n";

const char * const tick_emoji ="\
    000,000,000,000,000\n\
    000,000,000,000,255\n\
    000,000,000,255,000\n\
    255,000,255,000,000\n\
    000,255,000,000,000\n";

static bool led_on_off = false;

static void run_inferencing() {
    ei_impulse_result_t result = { 0 };
    signal_t features_signal;
    features_signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    features_signal.get_data = &raw_feature_get_data;

    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, true);
    uBit.serial.printf("run_classifier returned: %d\n", res);

    if (res != 0) return;

    uBit.serial.printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);

    bool is_microbit = false;

    // print the predictions
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (strcmp(result.classification[ix].label, "microbit") == 0 && result.classification[ix].value >= 0.3) {
            is_microbit = true;
        }
        uBit.serial.printf("    %s:\t%d\n", result.classification[ix].label, static_cast<int>(result.classification[ix].value * 100.0f));
    }
    uBit.serial.printf("\n\n");

    if (is_microbit) {
        MicroBitImage smile(happy_emoji);
        uBit.display.print(smile);
    }
    else {
        MicroBitImage smile(tick_emoji);
        uBit.display.print(smile);
    }

    uBit.display.image.setPixelValue(5, 5, led_on_off ? 100 : 0);
    led_on_off = !led_on_off;
}

/**
 * Creates a simple component that logs a stream of signed 16 bit data as signed 8-bit data over serial.
 * @param source a DataSource to measure the level of.
 * @param mode the format of the serialised data. Valid options are SERIAL_STREAM_MODE_BINARY (default), SERIAL_STREAM_MODE_DECIMAL, SERIAL_STREAM_MODE_HEX.
 */
SerialStreamer::SerialStreamer(DataSource &source, int mode) : upstream(source)
{
    this->mode = mode;

    // Register with our upstream component
    source.connect(*this);
}

/**
 * Callback provided when data is ready.
 */
int SerialStreamer::pullRequest()
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
ManagedBuffer SerialStreamer::getLastBuffer()
{
    return lastBuffer;
}

/**
 * Callback provided when data is ready.
 */
void SerialStreamer::streamBuffer(ManagedBuffer buffer)
{
    int CRLF = 0;
    int bps = upstream.getFormat();

    uint8_t *p = &buffer[0];
    uint8_t *end = p + buffer.length();

    while(p < end) {
        uint8_t v = *p++;
        ss_buffer[ss_buffer_ix++] = v;
        if (ss_buffer_ix >= EI_CLASSIFIER_RAW_SAMPLE_COUNT) {
            run_inferencing();
            ss_buffer_ix = 0;
            break;
        }
    }
    return;

    // // If a BINARY mode is requested, simply output all the bytes to the serial port.
    // if (mode == SERIAL_STREAM_MODE_BINARY)
    // {
    //     uint8_t *p = &buffer[0];
    //     uint8_t *end = p + buffer.length();

    //     while(p < end)
    //         uBit.serial.putc(*p++);
    // }

    // // if a HEX mode is requested, format using printf, framed by sample size..
    // if (mode == SERIAL_STREAM_MODE_HEX  || mode == SERIAL_STREAM_MODE_DECIMAL)
    // {
    //     uint8_t *d = &buffer[0];
    //     uint8_t *end = d+buffer.length();
    //     uint32_t data;

    //     while(d < end)
    //     {
    //         data = *d++;

    //         if (bps > DATASTREAM_FORMAT_8BIT_SIGNED)
    //             data |= (*d++) << 8;
    //         if (bps > DATASTREAM_FORMAT_16BIT_SIGNED)
    //             data |= (*d++) << 16;
    //         if (bps > DATASTREAM_FORMAT_24BIT_SIGNED)
    //             data |= (*d++) << 24;

    //         if (mode == SERIAL_STREAM_MODE_HEX)
    //             uBit.serial.printf("%x ", data);
    //         else
    //             uBit.serial.printf("%d ", data);

    //         CRLF++;

    //         if (CRLF == 16){
    //             uBit.serial.printf("\n");
    //             CRLF = 0;
    //         }
    //     }

    //     if (CRLF > 0)
    //         uBit.serial.printf("\n");
    // }
}
