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

#include "NRF52TouchSensor.h"
#include "CodalDmesg.h"

using namespace codal;

int lastTouchSample = 0;

static NRF52TouchSensor *instance = NULL;

static void touch_sense_irq(uint16_t mask)
{
    if (instance)
        instance->onSampleEvent();
}
/**
 * Constructor.
 *
 * Enables software controlled capacitative touch sensing on a set of pins.
 *
 * @param pin The physical pin on the device that drives the capacitative sensing.
 * @id The ID of this component, defaults to DEVICE_ID_TOUCH_SENSOR
 */
NRF52TouchSensor::NRF52TouchSensor(NRFLowLevelTimer& t, uint16_t id) : TouchSensor(id), timer(t)
{
    channel = 0;
    instance = this;

    // Configure as a fixed period timer for the required period    
    timer.setMode(TimerMode::TimerModeTimer);
    timer.setClockSpeed(16000);
    timer.setCompare(0, NRF52_TOUCH_SENSOR_PERIOD*16);

    // Use a PPI channel to capture a timestamp 
    NRF_PPI->CH[NRF52_TOUCH_SENSOR_PPI_CHANNEL].EEP = (uint32_t) &NRF_GPIOTE->EVENTS_IN[NRF52_TOUCH_SENSOR_GPIOTE_CHANNEL];   
    NRF_PPI->CH[NRF52_TOUCH_SENSOR_PPI_CHANNEL].TEP = (uint32_t) &timer.timer->TASKS_CAPTURE[1];
    NRF_PPI->CHENSET = 1 << NRF52_TOUCH_SENSOR_PPI_CHANNEL;

    // register for a low level interrupt when the timer matches CC0.
    timer.setIRQ(touch_sense_irq);

    // enable the timer.
    timer.enable();
    timer.enableIRQ();
}


/**
 * Begin touch sensing on the given button.
 */
int 
NRF52TouchSensor::addTouchButton(TouchButton *button)
{
    TouchSensor::addTouchButton(button);
    button->_pin.setDigitalValue(0);

    return DEVICE_OK;
}

extern void calibrateTest(float sample);

/**
 * Initiate a scan of the sensors.
 */
void 
NRF52TouchSensor::onSampleEvent()
{
    int result;

    // If we have no channels, to monitor then there's nothing to do.
    if (numberOfButtons == 0)
        return;

    // Capture the result from the last sense pass, and reset the capture value.
    result = timer.timer->CC[1];
    timer.timer->CC[1] = NRF52_TOUCH_SENSE_SAMPLE_MAX;

    // If we sensed a valid channel, configure the last pin used as an output, and start draining the pin.
    if (channel >= 0){

        // DEBUG
        //if (channel == 1)
        //    calibrateTest(result);

        buttons[channel]->setValue(result);
    }
    
    NRF_GPIOTE->CONFIG[NRF52_TOUCH_SENSOR_GPIOTE_CHANNEL] = 0;
    
    // Move on to the next channel. If we only have a single channel, then leave an empty timeslot for that
    // channel to drain all its charge before sampling it again.
    if (channel == 0 && numberOfButtons == 1)
        channel = -1;
    else
        channel = (channel + 1) % numberOfButtons;

    // Reset the timer, and enable the pin input event, unless we're leaving a timeslot for a single channel to charge.
    target_disable_irq();

    timer.timer->TASKS_CLEAR = 1;
    
    if (channel >= 0)
        NRF_GPIOTE->CONFIG[NRF52_TOUCH_SENSOR_GPIOTE_CHANNEL] = 0x00010001 | (buttons[channel]->_pin.name << 8);

    target_enable_irq();
}

