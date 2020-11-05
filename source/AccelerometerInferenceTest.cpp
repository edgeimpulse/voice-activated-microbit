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

#include "MicroBit.h"
#include "Tests.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"

/**
 * Invoked when we hear the keyword !
 */
static void heard_keyword() {
    const char * happy_emoji ="\
        000,255,000,255,000\n\
        000,000,000,000,000\n\
        255,000,000,000,255\n\
        000,255,255,255,000\n\
        000,000,000,000,000\n";
    MicroBitImage img(happy_emoji);
    uBit.display.print(img);
}

/**
 * Invoked when we hear something else
 */
static void heard_other() {
    const char * empty_emoji ="\
        000,000,000,000,000\n\
        000,000,000,000,000\n\
        000,000,255,000,000\n\
        000,000,000,000,000\n\
        000,000,000,000,000\n";
    MicroBitImage img(empty_emoji);
    uBit.display.print(img);
}

void
accelerometer_inference_test()
{

    ei_printf("\nStarting inferencing in 2 seconds...\n");

    uBit.sleep(2);

    while (1) {

        ei_printf("Sampling...\n");

        // Allocate a buffer here for the values we'll read from the IMU
        float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

        for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
            int x = uBit.accelerometer.getX();
            int y = uBit.accelerometer.getY();
            int z = uBit.accelerometer.getZ();

            buffer[ix + 0] = static_cast<float>(x) / 100.0f;
            buffer[ix + 1] = static_cast<float>(y) / 100.0f;
            buffer[ix + 2] = static_cast<float>(z) / 100.0f;

            uBit.sleep(EI_CLASSIFIER_INTERVAL_MS);
        }

        // Turn the raw buffer in a signal which we can the classify
        signal_t signal;
        int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
        if (err != 0) {
            ei_printf("Failed to create signal from buffer (%d)\n", err);
            return;
        }

        // Run the classifier
        ei_impulse_result_t result = { 0 };

        err = run_classifier(&signal, &result, true);
        if (err != EI_IMPULSE_OK) {
            ei_printf("ERR: Failed to run classifier (%d)\n", err);
            return;
        }

        bool is_updown = false;

        // print the predictions
        ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("    %s: %d\n", result.classification[ix].label, static_cast<int>(result.classification[ix].value * 1000.0f));
            if (strcmp(result.classification[ix].label, "updown") == 0 &&
                result.classification[ix].value > 0.8) {
                is_updown = true;
            }
        }
    #if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("    anomaly score: %d\n", static_cast<int>(result.anomaly * 1000.0f));
        if (result.anomaly > 0.3) {
            is_updown = false;
        }
    #endif

        if (is_updown) {
            heard_keyword();
        }
        else {
            heard_other();
        }

        uBit.sleep(2000);
    }
}

/**
 * Microbit implementations for Edge Impulse target-specific functions
 */
EI_IMPULSE_ERROR ei_sleep(int32_t time_ms) {
    uBit.sleep(time_ms);
    return EI_IMPULSE_OK;
}

void ei_printf(const char *format, ...) {
    char print_buf[1024] = { 0 };

    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);

    uBit.serial.printf("%s", print_buf);
}
