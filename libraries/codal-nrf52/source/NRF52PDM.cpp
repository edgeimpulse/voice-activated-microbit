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

#include "CodalCompat.h"
#include "NRF52PDM.h"
#include "nrf.h"

// Handle on the last (and probably only) instance of this class (NRF52 has only one PDM module)
static NRF52PDM *nrf52_pdm_driver = NULL;

static void nrf52_pdm_irq(void)
{
    // Simply pass on to the driver component handler.
    if (nrf52_pdm_driver)
        nrf52_pdm_driver->irq();
}

/**
 * Update our reference to a downstream component.
 * Pass through any connect requests to our output buffer component.
 *
 * @param component The new downstream component for this PDM audio source.
 */
void NRF52PDM::connect(DataSink& component)
{
    output.connect(component);
}

/**
 * Constructor for an instance of a PDM input (typically microphone),
 *
 * @param sd The pin the PDM data input is connected to.
 * @param sck The pin the PDM clock is conected to.
 * @param sampleRate the rate at which samples are generated in the output buffer (in Hz)
 * @param id The id to use for the message bus when transmitting events.
 */
NRF52PDM::NRF52PDM(Pin &sd, Pin &sck, uint16_t id) : output(*this)
{
    // Store our component ID.
    this->id = id;

    // Record a handle to this driver object, fo ruse by the IRQ handler.
    nrf52_pdm_driver = this;

    // Ensure the PDM module is disabled.
    this->disable();

    // Define our output stream as non-bocking.
    output.setBlocking(false);

    // Configure SCK as a digital output, and SD as digital input
    sck.setDigitalValue(0);
    sd.getDigitalValue();
    sd.setPull(PullMode::None);

    // Map the PDM peripheral onto the given pins...
    NRF_PDM->PSEL.CLK = sck.name;
    NRF_PDM->PSEL.DIN = sd.name;

    // Configure for DMA enabled, single channel PDM input.
    NVIC_SetVector(PDM_IRQn, (uint32_t )nrf52_pdm_irq);

    /* Set up interrupts */
    NRF_PDM->INTENSET = (PDM_INTENSET_STARTED_Enabled << PDM_INTENSET_STARTED_Pos) |
        (PDM_INTENSET_END_Enabled << PDM_INTENSET_END_Pos ) |
        (PDM_INTENSET_STOPPED_Enabled << PDM_INTENSET_STOPPED_Pos);

    NVIC_SetPriority( PDM_IRQn, 1);
    NVIC_ClearPendingIRQ( PDM_IRQn );
    NVIC_EnableIRQ( PDM_IRQn );


    // Configure for a 1.032MHz PDM clock.
    NRF_PDM->PDMCLKCTRL = (PDM_PDMCLKCTRL_FREQ_Default << PDM_PDMCLKCTRL_FREQ_Pos );

    // Mono operation.
    NRF_PDM->MODE = ( PDM_MODE_EDGE_LeftRising  << PDM_MODE_EDGE_Pos ) |
        (PDM_MODE_OPERATION_Mono << PDM_MODE_OPERATION_Pos);

    // Set default gain of 0dbm.
    this->setGain(40);


    // Configure buffer size - samples are 2 bytes each
    NRF_PDM->SAMPLE.MAXCNT = NRF52_PDM_BUFFER_SIZE / 2;

    // Record our sample rate for future computation.
    // This is a constant of the PDM samplerate / 64 (as defined in nrf52 specification, seciton 44).
    // For the configuration above, this translates to approx 16kHz.
    this->sampleRate = 1032000 / 64;
}

/**
 * Provide the next available ManagedBuffer to our downstream caller, if available.
 */
ManagedBuffer NRF52PDM::pull()
{
    return outputBuffer;
}

void NRF52PDM::irq()
{
    if (NRF_PDM->EVENTS_STARTED) {
        // We've just started receiving data the inputBuffer.
        // So we don't miss any data, line up the next buffer.
        // (unless we've been asked to stop).
        if (enabled)
            startDMA();

        NRF_PDM->EVENTS_STARTED = 0;
    }

    if (NRF_PDM->EVENTS_END)
    {
        if (outputBuffer.length() > 0)
            output.pullRequest();

        NRF_PDM->EVENTS_END = 0;
    }

    if (NRF_PDM->EVENTS_STOPPED)
        NRF_PDM->EVENTS_STOPPED = 0;

    // Erratum.
    // JF: Not sure what this is working around? Seems harmless, so leaving it in for now...
    (void) NRF_PDM->EVENTS_STOPPED;
}

/**
 * Define the gain level for the PDM input.
 *
 * @param gain Value in the range 0..80. This corresponds to -20db to +20db gain in 0.5db steps.
 * Default is 0dbm - a gain value of 40.
 */
int NRF52PDM::setGain(int gain)
{
    if (gain < 0 || gain > 50)
        return DEVICE_INVALID_PARAMETER;

    this->gain = gain;

    NRF_PDM->GAINL = (gain << PDM_GAINL_GAINL_Pos);
    NRF_PDM->GAINR = (gain << PDM_GAINR_GAINR_Pos);

    return DEVICE_OK;
}

/**
 * Enable this component
 */
void NRF52PDM::enable()
{
    if (!enabled)
    {
        enabled = true;

        NRF_PDM->ENABLE = 1;

        startDMA();

        NRF_PDM->TASKS_START = 1;
    }
}

/**
 * Disable this component
 */
void NRF52PDM::disable()
{
    // Schedule all DMA transfers to stop after the next DMA transaction completes.
    enabled = false;
    NRF_PDM->ENABLE = 0;
}

/**
 * Initiate a DMA transfer into the raw data buffer.
 */
void NRF52PDM::startDMA()
{
    outputBuffer = inputBuffer;
    inputBuffer = ManagedBuffer(NRF52_PDM_BUFFER_SIZE);

    NRF_PDM->SAMPLE.PTR = (uint32_t)&inputBuffer[0];
}
