/*
The MIT License (MIT)

Copyright (c) 2017 Lancaster University.
Copyright (c) 2018 Paul ADAM, inidinn.com

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
 * Class definition for a generic analog sensor, that takes the general form of a logarithmic response to a sensed value, in a potential divider.
 * Implements a base class for such a sensor, using the Steinhart-Hart equation to delineate a result.
 */

#include "Sensor.h"
#include "ErrorNo.h"
#include "CodalCompat.h"
#include "CodalFiber.h"
#include "Timer.h"

using namespace codal;

/**
 * Constructor.
 *
 * Creates a generic Sensor.
 *
 * @param id The ID of this compoenent e.g. DEVICE_ID_THERMOMETER
 */
Sensor::Sensor(uint16_t id, uint16_t sensitivity, uint16_t samplePeriod)
{
    this->id = id;
    this->setSensitivity(sensitivity);

    // Configure for a 2 Hz update frequency by default.
    if(EventModel::defaultEventBus)
        EventModel::defaultEventBus->listen(this->id, SENSOR_UPDATE_NEEDED, this, &Sensor::onSampleEvent, MESSAGE_BUS_LISTENER_IMMEDIATE);

    this->setPeriod(samplePeriod);
}

/*
 * Event Handler for periodic sample timer
 */
void Sensor::onSampleEvent(Event)
{
    updateSample();
}

/*
 * Determines the instantaneous value of the sensor, in SI units, and returns it.
 *
 * @return The current value of the sensor.
 */
int Sensor::getValue()
{
    return (int)sensorValue;
}

/**
 * Updates the internal reading of the sensor. Typically called periodically.
 */
void Sensor::updateSample()
{
    uint32_t value = readValue();

    // If this is the first reading performed, take it a a baseline. Otherwise, perform a decay average to smooth out the data.
    if (!(this->status & SENSOR_INITIALISED))
    {
        sensorValue = (uint16_t)value;
        this->status |=  SENSOR_INITIALISED;
    }
    else
    {
        sensorValue = ((sensorValue * (1023 - sensitivity)) + (value * sensitivity)) >> 10;
    }

    checkThresholding();
}

/**
 * Determine if any thresholding events need to be generated, and if so, raise them.
 */
void Sensor::checkThresholding()
{
    if ((this->status & SENSOR_HIGH_THRESHOLD_ENABLED) && (!(this->status & SENSOR_HIGH_THRESHOLD_PASSED)) && (sensorValue >= highThreshold))
    {
        Event(this->id, SENSOR_THRESHOLD_HIGH);
        this->status |=  SENSOR_HIGH_THRESHOLD_PASSED;
        this->status &= ~SENSOR_LOW_THRESHOLD_PASSED;
    }

    if ((this->status & SENSOR_LOW_THRESHOLD_ENABLED) && (!(this->status & SENSOR_LOW_THRESHOLD_PASSED)) && (sensorValue <= lowThreshold))

    {
        Event(this->id, SENSOR_THRESHOLD_LOW);
        this->status |=  SENSOR_LOW_THRESHOLD_PASSED;
        this->status &= ~SENSOR_HIGH_THRESHOLD_PASSED;
    }
}

/**
 * Set sensitivity value for the data. A decay average is taken of sampled data to smooth it into more accurate information.
 *
 * @param value A value between 0..1023 that detemrines the level of smoothing. Set to 1023 to disable smoothing. Default value is 868
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int Sensor::setSensitivity(uint16_t value)
{
    this->sensitivity = max(0, min(1023, value));

    return DEVICE_OK;
}

/**
 * Set the automatic sample period of the sensor to the specified value (in ms).
 *
 * @param period the requested time between samples, in milliseconds.
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int Sensor::setPeriod(int period)
{
    this->samplePeriod = period > 0 ? period : SENSOR_DEFAULT_SAMPLE_PERIOD;
    system_timer_event_every(this->samplePeriod, this->id, SENSOR_UPDATE_NEEDED);

    return DEVICE_OK;
}

/**
 * Reads the currently configured sample period.
 *
 * @return The time between samples, in milliseconds.
 */
int Sensor::getPeriod()
{
    return samplePeriod;
}

/**
 * Set threshold to the given value. Events will be generated when these thresholds are crossed.
 *
 * @param value the LOW threshold at which a SENSOR_THRESHOLD_LOW will be generated.
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int Sensor::setLowThreshold(uint16_t value)
{
    // Protect against churn if the same threshold is set repeatedly.
    if ((this->status & SENSOR_LOW_THRESHOLD_ENABLED) && lowThreshold == value)
        return DEVICE_OK;

    // We need to update our threshold
    lowThreshold = value;

    // Reset any exisiting threshold state, and enable threshold detection.
    this->status &= ~SENSOR_LOW_THRESHOLD_PASSED;
    this->status |=  SENSOR_LOW_THRESHOLD_ENABLED;

    // If a HIGH threshold has been set, ensure it's above the LOW threshold.
    if(this->status & SENSOR_HIGH_THRESHOLD_ENABLED)
        setHighThreshold(max(lowThreshold+1, highThreshold));

    return DEVICE_OK;
}

/**
 * Set threshold to the given value. Events will be generated when these thresholds are crossed.
 *
 * @param value the HIGH threshold at which a SENSOR_THRESHOLD_HIGH will be generated.
 *
 * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if the request fails.
 */
int Sensor::setHighThreshold(uint16_t value)
{
    // Protect against churn if the same threshold is set repeatedly.
    if ((this->status & SENSOR_HIGH_THRESHOLD_ENABLED) && highThreshold == value)
        return DEVICE_OK;

    // We need to update our threshold
    highThreshold = value;

    // Reset any exisiting threshold state, and enable threshold detection.
    this->status &= ~SENSOR_HIGH_THRESHOLD_PASSED;
    this->status |=  SENSOR_HIGH_THRESHOLD_ENABLED;

    // If a HIGH threshold has been set, ensure it's above the LOW threshold.
    if(this->status & SENSOR_LOW_THRESHOLD_ENABLED)
        setLowThreshold(min(highThreshold - 1, lowThreshold));

    return DEVICE_OK;
}

/**
 * Determines the currently defined low threshold.
 *
 * @return The current low threshold. DEVICE_INVALID_PARAMETER if no threshold has been defined.
 */
int Sensor::getLowThreshold()
{
    if (!(this->status & SENSOR_LOW_THRESHOLD_ENABLED))
        return DEVICE_INVALID_PARAMETER;

    return lowThreshold;
}

/**
 * Determines the currently defined high threshold.
 *
 * @return The current high threshold. DEVICE_INVALID_PARAMETER if no threshold has been defined.
 */
int Sensor::getHighThreshold()
{
    if (!(this->status & SENSOR_HIGH_THRESHOLD_ENABLED))
        return DEVICE_INVALID_PARAMETER;

    return highThreshold;
}

/**
 * Destructor.
 */
Sensor::~Sensor()
{
}
