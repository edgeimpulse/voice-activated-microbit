#include "NRF52PWM.h"
#include "nrf.h"
#include "cmsis.h"

#define  NRF52PWM_EMPTY_BUFFERSIZE  8
static uint16_t emptyBuffer[NRF52PWM_EMPTY_BUFFERSIZE];

void nrf52_pwm0_irq(void)
{
    // Simply pass on to the driver component handler.
    if (NRF52PWM::nrf52_pwm_driver[0])
        NRF52PWM::nrf52_pwm_driver[0]->irq();
}

void nrf52_pwm1_irq(void)
{
    // Simply pass on to the driver component handler.
    if (NRF52PWM::nrf52_pwm_driver[1])
        NRF52PWM::nrf52_pwm_driver[1]->irq();
}

void nrf52_pwm2_irq(void)
{
    // Simply pass on to the driver component handler.
    if (NRF52PWM::nrf52_pwm_driver[2])
        NRF52PWM::nrf52_pwm_driver[2]->irq();
}

// Handles on the instances of this class used the three PWM modules (if present)
NRF52PWM* NRF52PWM::nrf52_pwm_driver[NRF52PWM_PWM_PERIPHERALS] = { NULL };

NRF52PWM::NRF52PWM(NRF_PWM_Type *module, DataSource &source, float sampleRate, uint16_t id) : PWM(*module), upstream(source)
{
    // initialise state
    this->id = id;
    this->dataReady = 0;
    this->active = false;
    this->streaming = true;
    this->repeatOnEmpty = true;
    this->bufferPlaying = 0;
    this->stopStreamingAfterBuf = 0;

    // Clear empty buffer
    for (int i=0; i<NRF52PWM_EMPTY_BUFFERSIZE; i++)
        emptyBuffer[i] = 0x8000;

    // Ensure PWM is currently disabled.
    disable();

    // Configure hardware for requested sample rate.
    setSampleRate(sampleRate);

    // Configure for a repeating, edge aligned PWM pattern.
    PWM.MODE = PWM_MODE_UPDOWN_Up;

    // By default, enable control of four independent channels
    setDecoderMode(PWM_DECODER_LOAD_Individual);

    // Configure PWM for
    PWM.SEQ[1].REFRESH = 0;
    PWM.SEQ[0].REFRESH = 0;
    PWM.SEQ[1].ENDDELAY = 0;
    PWM.SEQ[0].ENDDELAY = 0;

    // Default to streaming mode.
    setStreamingMode(true);

    // Route an interrupt to this object
    // This is heavily unwound, but non trivial to remove this duplication given all the constants...
    // TODO: build up some lookup table to deduplicate this.
    if (&PWM == NRF_PWM0)
    {
        nrf52_pwm_driver[0] = this;
        NVIC_SetVector( PWM0_IRQn, (uint32_t) nrf52_pwm0_irq );
        NVIC_ClearPendingIRQ(PWM0_IRQn);
        NVIC_EnableIRQ(PWM0_IRQn);
    }

    if (&PWM == NRF_PWM1)
    {
        nrf52_pwm_driver[1] = this;
        NVIC_SetVector( PWM1_IRQn, (uint32_t) nrf52_pwm1_irq );
        NVIC_ClearPendingIRQ(PWM1_IRQn);
        NVIC_EnableIRQ(PWM1_IRQn);
    }

    if (&PWM == NRF_PWM2)
    {
        nrf52_pwm_driver[2] = this;
        NVIC_SetVector( PWM2_IRQn, (uint32_t) nrf52_pwm2_irq );
        NVIC_ClearPendingIRQ(PWM2_IRQn);
        NVIC_EnableIRQ(PWM2_IRQn);
    }

    // Enable the PWM module
    enable();

    // Register with our upstream component
    upstream.connect(*this);
}

/**
 * Determine the DAC playback sample rate to the given frequency.
 * @return the current sample rate.
 */
float NRF52PWM::getSampleRate()
{
    return sampleRate;
}

/**
 * Determine the maximum unsigned vlaue that can be loaded into the PWM data values, for the current
 * frequency configuration.
 */
int NRF52PWM::getSampleRange()
{
    return PWM.COUNTERTOP;
}

/**
 * Change the DAC playback sample rate to the given frequency.
 * @param frequency The new sample playback frequency.
 */
float NRF52PWM::setSampleRate(float frequency)
{
    return setPeriodUs(1000000 / frequency);
}

/**
 * Change the DAC playback sample rate to the given period.
 * @param period The new sample playback period, in microseconds.
 */
int NRF52PWM::setPeriodUs(float period)
{
    int prescaler = 0;
    int clock_frequency = 16000000;
    int period_ticks = (clock_frequency/1000000) * period;

    // Calculate necessary prescaler.
    while(period_ticks >> prescaler >= 32768)
        prescaler++;

    // If the sample rate requested is outside the range of what the hardware can achieve, then there's nothign we can do.
    if (prescaler > 7)
        return DEVICE_INVALID_PARAMETER;

    // Decrement prescaler, as hardware indexes from zero.
    PWM.PRESCALER = prescaler;
    PWM.COUNTERTOP = period_ticks >> prescaler;

    // Update our internal record to reflect an accurate (probably rounded) samplerate.
    period_ticks = period_ticks >> prescaler;
    period_ticks = period_ticks << prescaler;

    periodUs = period_ticks / (clock_frequency/1000000);
    sampleRate = 1000000 / periodUs;

    return DEVICE_OK;
}

/**
 * Determine the current DAC playback period.
 * @return period The sample playback period, in microseconds.
 */
float NRF52PWM::getPeriodUs()
{
    return periodUs;
}


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
int NRF52PWM::setDecoderMode(uint32_t mode)
{
    PWM.DECODER = (mode << PWM_DECODER_LOAD_Pos ) | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos );

    return DEVICE_OK;
}
 
/**
 * Defines if the PWM module should maintain playout ordering of buffers, or always play the most recent buffer provided.
 * 
 * @ param streamingMode If true, buffers will be streamed in order they are received. If false, the most recent buffer supplied always takes prescedence.
 * @ param repeatOnEmpty If set to true, the last buffer received will be repeated when no data is available. If false, the PWM channel will be suspended.
 */
void NRF52PWM::setStreamingMode(bool streamingMode, bool repeatOnEmpty)
{
    this->streaming = streamingMode;
    this->repeatOnEmpty = repeatOnEmpty;

    if (streaming)
    {
        PWM.LOOP = 1;
        PWM.SHORTS = PWM_SHORTS_LOOPSDONE_SEQSTART0_Enabled << PWM_SHORTS_LOOPSDONE_SEQSTART0_Pos; 
        PWM.INTENSET = (PWM_INTEN_SEQEND0_Enabled << PWM_INTEN_SEQEND0_Pos ) | (PWM_INTEN_SEQEND1_Enabled << PWM_INTEN_SEQEND1_Pos);
    }
    else
    {
        PWM.LOOP = 0;
        PWM.SHORTS = 0; 
        PWM.INTENCLR = (PWM_INTEN_SEQEND0_Enabled << PWM_INTEN_SEQEND0_Pos ) | (PWM_INTEN_SEQEND1_Enabled << PWM_INTEN_SEQEND1_Pos);
    }
}

/**
 * Pull a buffer into the given double buffer slot, if one is available.
 * @param b The buffer to fill (either 0 or 1)
 * @return the number of buffers succesfully filled.
 */
int NRF52PWM::tryPull(uint8_t b)
{
    if (stopStreamingAfterBuf)
    {
        PWM.TASKS_STOP = 1;
        while(PWM.EVENTS_STOPPED == 0);

        active = false;
        bufferPlaying = 0;
        stopStreamingAfterBuf = 0;

        // If a Pull request has been made since we decided to stop, start to fill up the
        // hardware double buffer so that we don't stall.
        if(dataReady)
        {
            dataReady--;
            pullRequest();
        }
        return 0;
    }

    if (dataReady){
        buffer[b] = upstream.pull();
        PWM.SEQ[b].PTR = (uint32_t) buffer[b].getBytes();
        PWM.SEQ[b].CNT = buffer[b].length() / 2;

        dataReady--;

        return 1;
    }

    // If we're in active streaming mode, and have requested a buffer and failed to get one, we have an underflow.
    // Streaming mode is double buffered, so schedule ourself to stop after the next buffer is played, if we're so configured.
    if (streaming && active && !repeatOnEmpty)
    {
        // The PWM doesn't seem to respond to changes in the SHORTS register while it's active...
        // instead, we provide an empty buffer to prevent partial repetition of any previous buffer.
        PWM.SEQ[b].PTR = (uint32_t) emptyBuffer;
        PWM.SEQ[b].CNT = (uint32_t) NRF52PWM_EMPTY_BUFFERSIZE;
        stopStreamingAfterBuf = 1;
    }
    return 0;
}

/**
 * Callback provided when data is ready.
 */
int NRF52PWM::pullRequest()
{
    dataReady++;

    // If we're not running in streaming mode, simply pull the requested buffer and schedule for DMA.
    if (!streaming)
    {
        int result = tryPull(0);
        if (result || repeatOnEmpty)
            PWM.TASKS_SEQSTART[0] = 1;
    }

    // If we're in streaming mode, ensure that we've preloaded both double buffers before initiating playout.
    // note: care needed here, as our upstream data source MAY recursively call pullRequest() again in response to us
    // pulling the first buffer...
    if (streaming && !active)
    {
        active = true;

        tryPull(bufferPlaying);
        bufferPlaying = (bufferPlaying + 1) % 2;

        if (bufferPlaying !=0 && dataReady)
        {
            tryPull(bufferPlaying);
            bufferPlaying = (bufferPlaying + 1) % 2;
        }

        // Check if we've preloaded both buffers
        if (bufferPlaying == 0)
            PWM.TASKS_SEQSTART[0] = 1;
        else
            active = false;
    }

    return DEVICE_OK;
}

/**
 * Base implementation of a DMA callback
 */
void NRF52PWM::irq()
{
    // once the sequence has finished playing, load up the next buffer.
    if (PWM.EVENTS_SEQEND[0])
    {
        bufferPlaying = 1;
        tryPull(0);

        PWM.EVENTS_SEQEND[0] = 0;
    }

    if (PWM.EVENTS_SEQEND[1])
    {
        bufferPlaying = 0;
        tryPull(1);

        PWM.EVENTS_SEQEND[1] = 0;
    }
}

/**
 * Enable this component
 */
void NRF52PWM::enable()
{
    enabled = true;
    PWM.ENABLE = 1;
}

/**
 * Disable this component
 */
void NRF52PWM::disable()
{
    enabled = false;
    PWM.ENABLE = 0;
}

/**
 * Direct output of given PWM channel to the given pin
 */
int
NRF52PWM::connectPin(Pin &pin, int channel)
{
    if (channel >= NRF52PWM_PWM_CHANNELS)
        return DEVICE_INVALID_PARAMETER;

    // If the pin is already connected to the requested channel, we have nothing to do.
    // We optimise this in order to prevent any possible glitch to an already connected channel...
    if (PWM.PSEL.OUT[channel] == pin.name)
        return DEVICE_OK;

    pin.disconnect();
    pin.setDigitalValue(0);
    PWM.PSEL.OUT[channel] = pin.name;
    pin.disconnect();

    pin.status |= IO_STATUS_ANALOG_OUT;
    return DEVICE_OK;
}

/**
 * Direct output of given PWM channel to the given pin
 */
int
NRF52PWM::disconnectPin(Pin &pin)
{
    for (int channel = 0; channel < NRF52PWM_PWM_CHANNELS; channel++)
        if (PWM.PSEL.OUT[channel] == pin.name)
            PWM.PSEL.OUT[channel] = 0xFFFFFFFF;

    pin.status &= ~IO_STATUS_ANALOG_OUT;
    return DEVICE_OK;
}