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

/**
 * Class definition for a normalised, non-linear analog sensor, that takes the general form of a logarithmic response to a sensed value, in a potential divider.
 * Implements a base class for such a sensor, using the Steinhart-Hart equation to delineate a result.
 */

#include "NonLinearAnalogSensor.h"

using namespace codal;

/**
 * Constructor.
 *
 * Creates a generic AnalogSensor.
 *
 * @param pin The pin on which to sense
 * @param nominalValue The value (in SI units) of a nominal position.
 * @param nominalReading The raw reading from the sensor at the nominal position.
 * @param beta The Steinhart-Hart Beta constant for the device
 * @param seriesResistor The value (in ohms) of the resistor in series with the sensor.
 * @param zeroOffset Optional zero offset applied to all SI units (e.g. 273.15 for temperature sensing in C vs Kelvin).
 */
NonLinearAnalogSensor::NonLinearAnalogSensor(Pin &pin, uint16_t id, float nominalValue, float nominalReading, float beta, float seriesResistor, float zeroOffset) : AnalogSensor(pin, id)
{
    this->nominalValue = nominalValue;
    this->nominalReading = nominalReading;
    this->beta = beta;
    this->seriesResistor = seriesResistor;
    this->zeroOffset = zeroOffset;
}

/**
 * Updates the internal reading of the sensor. Typically called periodicaly.
 *
 * @return DEVICE_OK on success.
 */
void NonLinearAnalogSensor::updateSample()
{
    float sensorReading;
    float value;

    sensorReading = (((1023.0f) * this->seriesResistor) / this->readValue()) - this->seriesResistor;
    value = (1.0f / ((log(sensorReading / this->nominalReading) / this->beta) + (1.0f / (this->nominalValue + this->zeroOffset)))) - this->zeroOffset;

    // If this is the first reading performed, take it a a baseline. Otherwise, perform a decay average to smooth out the data.
    if (!(status & ANALOG_SENSOR_INITIALISED))
    {
        this->sensorValue = value;
        this->status |=  ANALOG_SENSOR_INITIALISED;
    }
    else
    {
        this->sensorValue = ((this->sensorValue * (1023 - this->sensitivity)) + ((uint16_t)value * this->sensitivity)) >> 10;
    }

    checkThresholding();
}
