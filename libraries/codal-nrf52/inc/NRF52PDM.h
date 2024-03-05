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

#include "CodalConfig.h"
#include "codal-core/inc/types/Event.h"
#include "Pin.h"
#include "DataStream.h"

#ifndef NRF52PDM_H
#define NRF52PDM_H

//
// Event codes
//
#define NRF52_PDM_DATA_READY            1

//
// Constants
//
#define NRF52_PDM_BUFFER_SIZE           512

using namespace codal;

class NRF52PDM : public CodalComponent, public DataSource
{

private:
    bool            enabled;                                // Determines if this component is actively receiving data.
	ManagedBuffer   inputBuffer, outputBuffer;              // A reference counted stream buffer used to hold PCM sample data.
    uint32_t        outputBufferSize;                       // The size of our output buffer.
	uint32_t        sampleRate;                             // The PCM output target sample rate (in bps).
    uint8_t         gain;                                   // The gain to apply to the pdm input channels.

public:

	DataStream output;

    /**
      * Constructor for an instance of a PDM input (typically microphone),
      *
      * @param sd The pin the PDM data input is connected to.
      * @param sck The pin the PDM clock is conected to.
      * @param sampleRate the rate at which samples are generated in the output buffer (in Hz)
      * @param id The id to use for the message bus when transmitting events.
      */
    NRF52PDM(Pin &sd, Pin &sck, uint16_t id = DEVICE_ID_SYSTEM_MICROPHONE);

	/**
	 * Provide the next available ManagedBuffer to our downstream caller, if available.
	 */
	virtual ManagedBuffer pull();

	/**
     * Update our reference to a downstream component.
	 */
	virtual void connect(DataSink &sink);

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
     * Define the gain level for the PDM input.
     *
     * @param gain Value in the range 0..80. This corresponds to -20db to +20db gain in 0.5db steps. 
     * Default is 0dbm - a gain value of 40.
     */
    int setGain(int gain);

private:

    void startDMA();
};

#endif
