#include "MicroBit.h"
#include "ContinuousAudioStreamer.h"
#include "StreamNormalizer.h"
#include "LevelDetector.h"
#include "LevelDetectorSPL.h"
#include "Tests.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"

static NRF52ADCChannel *mic = NULL;
static ContinuousAudioStreamer *streamer = NULL;
static StreamNormalizer *processor = NULL;
static LevelDetector *level = NULL;
static LevelDetectorSPL *levelSPL = NULL;
static int claps = 0;
static volatile int sample;

static inference_t inference;

/**
 * Get raw audio signal data
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int8_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);

    return 0;
}


void
onLoud(MicroBitEvent)
{
    uBit.serial.printf("LOUD\n");
    claps++;
    if (claps >= 10)
        claps = 0;

    uBit.display.print(claps);
}

void
onQuiet(MicroBitEvent)
{
    uBit.serial.printf("QUIET\n");
}

void mems_mic_drift_test()
{
    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    while(true)
    {
        sample = uBit.io.P0.getAnalogValue();
        uBit.sleep(250);

        sample = uBit.io.microphone.getAnalogValue();
        uBit.sleep(250);

        uBit.display.scroll(sample);
    }
}

void
mems_mic_test()
{
    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,0);          // Uncomment for v1.47.2
        //mic->setGain(7,1);        // Uncomment for v1.46.2
    }

    // alloc inferencing buffers
    inference.buffers[0] = (int8_t *)malloc(EI_CLASSIFIER_SLICE_SIZE * sizeof(int8_t));

    if (inference.buffers[0] == NULL) {
        uBit.serial.printf("Failed to alloc buffer 1\n");
        return;
    }

    inference.buffers[1] = (int8_t *)malloc(EI_CLASSIFIER_SLICE_SIZE * sizeof(int8_t));

    if (inference.buffers[0] == NULL) {
        uBit.serial.printf("Failed to alloc buffer 2\n");
        free(inference.buffers[0]);
        return;
    }

    uBit.serial.printf("Allocated buffers\n");

    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = EI_CLASSIFIER_SLICE_SIZE;
    inference.buf_ready = 0;

    mic->output.setBlocking(true);

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 0.05f, true, DATASTREAM_FORMAT_8BIT_SIGNED);

    if (streamer == NULL)
        streamer = new ContinuousAudioStreamer(processor->output, &inference);

    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    uBit.serial.printf("Allocated everything else\n");

    while(1) {
        uBit.sleep(1);

        if (inference.buf_ready) {
            inference.buf_ready = 0;

            static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);

            signal_t signal;
            signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
            signal.get_data = &microphone_audio_signal_get_data;
            ei_impulse_result_t result = { 0 };

            EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, false);
            if (r != EI_IMPULSE_OK) {
                uBit.serial.printf("ERR: Failed to run classifier (%d)\n", r);
                return;
            }

            if (++print_results >= 0) {
                // print the predictions
                uBit.serial.printf("Predictions (DSP: %d ms., Classification: %d ms.): \n",
                        result.timing.dsp, result.timing.classification);
                for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                    uBit.serial.printf("    %s: %d\n", result.classification[ix].label,
                            static_cast<int>(result.classification[ix].value * 1000.0f));
                }
            }
        }
    }
}

void
mems_clap_test(int wait_for_clap)
{
    claps = 0;

    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,0);
    }

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 1.0f, true, DATASTREAM_FORMAT_UNKNOWN, 10);

    if (level == NULL)
        level = new LevelDetector(processor->output, 150, 75);

    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3))
        uBit.sleep(1000);

    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_SYSTEM_LEVEL_DETECTOR, LEVEL_THRESHOLD_LOW, onQuiet);
}

void
mems_clap_test_spl(int wait_for_clap)
{
    claps = 0;

    if (mic == NULL){
        mic = uBit.adc.getChannel(uBit.io.microphone);
        mic->setGain(7,0);
    }

    if (processor == NULL)
        processor = new StreamNormalizer(mic->output, 1.0f, true, DATASTREAM_FORMAT_UNKNOWN, 10);

    if (levelSPL == NULL)
        levelSPL = new LevelDetectorSPL(processor->output, 75.0, 60.0, 9, 52, DEVICE_ID_MICROPHONE);

    uBit.io.runmic.setDigitalValue(1);
    uBit.io.runmic.setHighDrive(true);

    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_LOW, onQuiet);

    while(!wait_for_clap || (wait_for_clap && claps < 3))
        uBit.sleep(1000);

    uBit.messageBus.ignore(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.ignore(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_LOW, onQuiet);
}

class MakeCodeMicrophoneTemplate {
  public:
    MIC_DEVICE microphone;
    LevelDetectorSPL level;
    MakeCodeMicrophoneTemplate() MIC_INIT { MIC_ENABLE; }
};

void
mc_clap_test()
{
    new MakeCodeMicrophoneTemplate();

    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_HIGH, onLoud);
    uBit.messageBus.listen(DEVICE_ID_MICROPHONE, LEVEL_THRESHOLD_LOW, onQuiet);

    while(1)
    {
        uBit.sleep(1000);
    }
}
