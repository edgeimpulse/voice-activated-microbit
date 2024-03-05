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

#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "CodalConfig.h"
#include "Pin.h"
#include "Event.h"
#include "Sensor.h"

/**
  * Sensor events deprecated, use SENSOR_* instead
  */
#define ANALOG_THRESHOLD_LOW SENSOR_THRESHOLD_LOW
#define ANALOG_THRESHOLD_HIGH SENSOR_THRESHOLD_HIGH
#define ANALOG_SENSOR_UPDATE_NEEDED SENSOR_UPDATE_NEEDED

/**
 * Status values deprecated, use SENSOR_* instead
 */
#define ANALOG_SENSOR_INITIALISED SENSOR_INITIALISED
#define ANALOG_SENSOR_HIGH_THRESHOLD_PASSED SENSOR_HIGH_THRESHOLD_PASSED
#define ANALOG_SENSOR_LOW_THRESHOLD_PASSED SENSOR_LOW_THRESHOLD_PASSED
#define ANALOG_SENSOR_LOW_THRESHOLD_ENABLED SENSOR_LOW_THRESHOLD_ENABLED
#define ANALOG_SENSOR_HIGH_THRESHOLD_ENABLED SENSOR_HIGH_THRESHOLD_ENABLED

namespace codal
{
    /**
     * Class definition for a generic analog sensor, and performs periodic sampling, buffering and low pass filtering of the data.
     */
    class AnalogSensor : public Sensor
    {
        private:

        Pin& pin;              // Pin where the sensor is connected.

        public:

        /**
          * Constructor.
          *
          * Creates a generic AnalogSensor.
          *
          * @param pin The pin on which to sense
          * @param id The ID of this compoenent e.g. DEVICE_ID_THERMOMETER
         */
        AnalogSensor(Pin &pin, uint16_t id);

        /**
         * Read the value from pin.
         */
        virtual int readValue();

        /**
          * Destructor.
          */
        ~AnalogSensor();

   };
}

#endif
