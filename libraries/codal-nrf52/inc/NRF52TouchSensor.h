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

#ifndef NRF52_TOUCH_SENSOR_H
#define NRF52_TOUCH_SENSOR_H

#include "CodalConfig.h"
#include "CodalComponent.h"
#include "Event.h"
#include "Pin.h"
#include "TouchSensor.h"
#include "NRFLowLevelTimer.h"

#define NRF52_TOUCH_SENSOR_PERIOD           1000                             // The period between each sensing cycle (uS)
#define NRF52_TOUCH_SENSE_SAMPLE_MAX        (NRF52_TOUCH_SENSOR_PERIOD*16)   // The maximum sample value returned (if the pin never raises)
#define NRF52_TOUCH_SENSOR_PPI_CHANNEL      2           
#define NRF52_TOUCH_SENSOR_GPIOTE_CHANNEL   0


namespace codal
{
    /**
      * Class definition for an NRF52TouchSensor
      *
      * Drives a number of single ended TouchButtons, based on a hardware supported implementation unsing a hardware timer and PPI.
      */
    class NRF52TouchSensor : public TouchSensor
    {
        NRFLowLevelTimer&   timer;              // The timer module used to capture results.
        int                 channel;            // The channel being sampled.

        public:

        /**
          * Constructor.
          *
          * Enables software controlled capacitative touch sensing on a set of pins.
          *
          * @param pin The physical pin on the device that drives the capacitative sensing.
          * @id The ID of this component, defaults to DEVICE_ID_TOUCH_SENSOR
          */
        NRF52TouchSensor(NRFLowLevelTimer& t, uint16_t id = DEVICE_ID_TOUCH_SENSOR);

        /**
          * Begin touch sensing on the given button
          */
        virtual int addTouchButton(TouchButton *button);

        /**
         * Initiate a scan of the sensors.
         */
        void onSampleEvent();

    };
}

#endif
