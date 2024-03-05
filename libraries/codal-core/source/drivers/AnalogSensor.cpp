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

#include "AnalogSensor.h"

/**
 * Class definition for a generic analog sensor, that takes the general form of a logarithmic response to a sensed value, in a potential divider.
 * Implements a base class for such a sensor, using the Steinhart-Hart equation to delineate a result.
 */

using namespace codal;

/**
 * Constructor.
 *
 * Creates a generic AnalogSensor.
 *
 * @param pin The pin on which to sense
 * @param id The ID of this component e.g. DEVICE_ID_THERMOMETER
 */
AnalogSensor::AnalogSensor(Pin &pin, uint16_t id) : Sensor( id), pin(pin)
{
    updateSample();
}

/**
 * Read the value from pin.
 */
int AnalogSensor::readValue()
{
    return this->pin.getAnalogValue();
}

/**
 * Destructor.
 */
AnalogSensor::~AnalogSensor()
{
}