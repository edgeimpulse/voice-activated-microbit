#include "CodalConfig.h"
#include "codal-core/inc/types/Event.h"
#include "Timer.h"
#include "NRF52Pin.h"
#include "DataStream.h"
#include "nrf.h"

#ifndef NRF52PWM_H
#define NRF52PWM_H

#ifndef NRF52PWM_DEFAULT_FREQUENCY
#define NRF52PWM_DEFAULT_FREQUENCY 16000
#endif

#define NRF52PWM_PWM_PERIPHERALS    3
#define NRF52PWM_PWM_CHANNELS       4


using namespace codal;

class NRF52PWM : public CodalComponent, public DataSink
{

private:
    NRF_PWM_Type    &PWM;                   // The hardware PWM module used by this instance
    bool            enabled;                // Determines if this PWM instance is enabled
    bool            active;                 // Determines if this PWM instance is actively generating output
    bool            streaming;              // Determines if the output is streamed, or discrete. Streamed mode maintains ordered, discrete repeats playout most recent data provided.
    bool            repeatOnEmpty;          // Determines the behaviour of the PWM if a buffer underflow occurs.
    int             dataReady;              // Count of the number of input buffers awaiting playout
    float           sampleRate;
    float           periodUs;               // Period between output samples, in microseconds
    uint8_t         bufferPlaying;          // ID of the buffer currently being played (0 or 1). Output is hardware double buffered.
    int8_t          stopStreamingAfterBuf;  // When stopping, the last buffer ID to play beforhand. -1 if no stop is scheduled.
    ManagedBuffer   buffer[2];              // The ManagedBuffers currently being used by the PWN hardware

public:

    // The stream component that is serving our data
    DataSource      &upstream;

    // Handles on the instances of this class used the three PWM modules (if present)
    static NRF52PWM *nrf52_pwm_driver[NRF52PWM_PWM_PERIPHERALS];
    

    /**
      * Constructor for an instance of a PWM acting as a sink to a given (likely DMA enabled) datastream.
      * Use this constructor to create a PWM output who's values are controlled dynamically by an asynchronous data stream.
      *
      * @param source The DataSource that will provide data.
      * @param sampleRate The frequency (in Hz) that data will be presented.
      * @param id The id to use for the message bus when transmitting events.
      */
    NRF52PWM(NRF_PWM_Type *module, DataSource &source, float sampleRate = NRF52PWM_DEFAULT_FREQUENCY, uint16_t id = DEVICE_ID_SYSTEM_DAC);

    /**
     * Callback provided when data is ready.
     */
    virtual int pullRequest();

    /**
     * Determine the DAC playback sample rate to the given frequency.
     * @return the current sample rate.
     */
    float getSampleRate();

    /**
     * Determine the maximum unsigned vlaue that can be loaded into the PWM data values, for the current
     * frequency configuration.
     */
    int getSampleRange();

    /**
     * Change the DAC playback sample rate to the given frequency.
     * @param frequency The new sample playback frequency.
     */
    float setSampleRate(float frequency);

    /**
     * Determine the current DAC playback period.
     * @return period The sample playback period, in microseconds.
     */
    float getPeriodUs();


    /**
     * Change the DAC playback sample rate to the given period.
     * @param period The new sample playback period, in microseconds.
     */
    int setPeriodUs(float period);

    /** 
     * Defines the mode in which the PWM module will operate, in terms of how it interprets data provided from the DataSource:
     * Valid options are:
     * 
     * PWM_DECODER_LOAD_Common          1st half word (16-bit) used in all PWM channels 0..3 
     * PWM_DECODER_LOAD_Grouped         1st half word (16-bit) used in channel 0..1; 2nd word in channel 2..3
     * PWM_DECODER_LOAD_Individual      1st half word (16-bit) in ch.0; 2nd in ch.1; ...; 4th in ch.3 
     * PWM_DECODER_LOAD_WaveForm        1st half word (16-bit) in ch.0; 2nd in ch.1; ...; 4th in COUNTERTOP
     * 
     * (See nrf52 product specificaiton for more details)
     * 
     * @param mode The mode for this PWM module to use.
     * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
     */
    int setDecoderMode(uint32_t mode);
 
    /**
     * Defines if the PWM module should maintain playout ordering of buffers, or always play the most recent buffer provided.
     * 
     * @ param streamingMode If true, buffers will be streamed in order they are received. If false, the most recent buffer supplied always takes prescedence.
     * @ param repeatOnEmpty If set to true, the last buffer received will be repeated when no data is available. If false, the PWM channel will be suspended.
     */
    void setStreamingMode(bool streamingMode, bool repeatOnEmpty = true);

    /**
     * Interrupt callback when playback of DMA buffer has completed
     */
    void irq();

    /**
     * Enable this component
     */
    void enable();

    /**
     * Disable this component
     */
    void disable();

    /**
     * Direct output of given PWM channel to the given pin
     */
    int
    connectPin(Pin &pin, int channel);

    /**
     * Disconnect the given pin from any PWM channels it has allocated.
     */
    int
    disconnectPin(Pin &pin);

    private:
    /**
     * Pull a buffer into the given double buffer slot, if one is available.
     */
    int tryPull(uint8_t b);

};

#endif
