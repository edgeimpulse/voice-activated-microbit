/*
The MIT License (MIT)

Copyright (c) 2017 Lancaster University.
Copyright (c) 2018 Paul ADAM, Europe.

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

#include "Gyroscope.h"
#include "ErrorNo.h"
#include "Event.h"
#include "CodalCompat.h"
#include "CodalFiber.h"

using namespace codal;


/**
  * Constructor.
  * Create a software abstraction of an FXSO8700 combined accelerometer/magnetometer
  *
  * @param _i2c an instance of I2C used to communicate with the device.
  *
  * @param address the default I2C address of the accelerometer. Defaults to: FXS8700_DEFAULT_ADDR.
  *
 */
Gyroscope::Gyroscope(CoordinateSpace &cspace, uint16_t id) : sample(), sampleENU(), coordinateSpace(cspace)
{
    // Store our identifiers.
    this->id = id;
    this->status = 0;

    // Set a default rate of 50Hz and a +/-2g range.
    this->samplePeriod = 20;
    this->sampleRange = 2;
}

/**
  * Stores data from the accelerometer sensor in our buffer, and perform gesture tracking.
  *
  * On first use, this member function will attempt to add this component to the
  * list of fiber components in order to constantly update the values stored
  * by this object.
  *
  * This lazy instantiation means that we do not
  * obtain the overhead from non-chalantly adding this component to fiber components.
  *
  * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the read request fails.
  */
int Gyroscope::update(Sample3D s)
{
    // Store the new data, after performing any necessary coordinate transformations.
    sampleENU = s;
    sample = coordinateSpace.transform(s);

    // Indicate that pitch and roll data is now stale, and needs to be recalculated if needed.
    status &= ~GYROSCOPE_IMU_DATA_VALID;

    // Indicate that a new sample is available
    Event e(id, GYROSCOPE_EVT_DATA_UPDATE);

    return DEVICE_OK;
};

/**
  * A service function.
  * It calculates the current scalar acceleration of the device (x^2 + y^2 + z^2).
  * It does not, however, square root the result, as this is a relatively high cost operation.
  *
  * This is left to application code should it be needed.
  *
  * @return the sum of the square of the acceleration of the device across all axes.
  */
uint32_t Gyroscope::instantaneousAccelerationSquared()
{
    requestUpdate();

    // Use pythagoras theorem to determine the combined force acting on the device.
    return (uint32_t)sample.x*(uint32_t)sample.x + (uint32_t)sample.y*(uint32_t)sample.y + (uint32_t)sample.z*(uint32_t)sample.z;
}

/**
  * Attempts to set the sample rate of the accelerometer to the specified value (in ms).
  *
  * @param period the requested time between samples, in milliseconds.
  *
  * @return DEVICE_OK on success, DEVICE_I2C_ERROR is the request fails.
  *
  * @code
  * // sample rate is now 20 ms.
  * accelerometer.setPeriod(20);
  * @endcode
  *
  * @note The requested rate may not be possible on the hardware. In this case, the
  * nearest lower rate is chosen.
  */
int Gyroscope::setPeriod(int period)
{
    int result;

    samplePeriod = period;
    result = configure();

    samplePeriod = getPeriod();
    return result;

}

/**
  * Reads the currently configured sample rate of the accelerometer.
  *
  * @return The time between samples, in milliseconds.
  */
int Gyroscope::getPeriod()
{
    return (int)samplePeriod;
}

/**
  * Attempts to set the sample range of the accelerometer to the specified value (in g).
  *
  * @param range The requested sample range of samples, in g.
  *
  * @return DEVICE_OK on success, DEVICE_I2C_ERROR is the request fails.
  *
  * @code
  * // the sample range of the accelerometer is now 8G.
  * accelerometer.setRange(8);
  * @endcode
  *
  * @note The requested range may not be possible on the hardware. In this case, the
  * nearest lower range is chosen.
  */
int Gyroscope::setRange(int range)
{
    int result;

    sampleRange = range;
    result = configure();

    sampleRange = getRange();
    return result;
}

/**
  * Reads the currently configured sample range of the accelerometer.
  *
  * @return The sample range, in g.
  */
int Gyroscope::getRange()
{
    return (int)sampleRange;
}

/**
 * Configures the accelerometer for G range and sample rate defined
 * in this object. The nearest values are chosen to those defined
 * that are supported by the hardware. The instance variables are then
 * updated to reflect reality.
 *
 * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the accelerometer could not be configured.
 *
 * @note This method should be overidden by the hardware driver to implement the requested
 * changes in hardware.
 */
int Gyroscope::configure()
{
    return DEVICE_NOT_SUPPORTED;
}

/**
 * Poll to see if new data is available from the hardware. If so, update it.
 * n.b. it is not necessary to explicitly call this function to update data
 * (it normally happens in the background when the scheduler is idle), but a check is performed
 * if the user explicitly requests up to date data.
 *
 * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the update fails.
 *
 * @note This method should be overidden by the hardware driver to implement the requested
 * changes in hardware.
 */
int Gyroscope::requestUpdate()
{
    return DEVICE_NOT_SUPPORTED;
}

/**
 * Reads the last accelerometer value stored, and provides it in the coordinate system requested.
 *
 * @param coordinateSpace The coordinate system to use.
 * @return The force measured in each axis, in milli-g.
 */
Sample3D Gyroscope::getSample(CoordinateSystem coordinateSystem)
{
    requestUpdate();
    return coordinateSpace.transform(sampleENU, coordinateSystem);
}

/**
 * Reads the last accelerometer value stored, and in the coordinate system defined in the constructor.
 * @return The force measured in each axis, in milli-g.
 */
Sample3D Gyroscope::getSample()
{
    requestUpdate();
    return sample;
}

/**
 * reads the value of the x axis from the latest update retrieved from the accelerometer,
 * usingthe default coordinate system as specified in the constructor.
 *
 * @return the force measured in the x axis, in milli-g.
 */
int Gyroscope::getX()
{
    requestUpdate();
    return sample.x;
}

/**
 * reads the value of the y axis from the latest update retrieved from the accelerometer,
 * usingthe default coordinate system as specified in the constructor.
 *
 * @return the force measured in the y axis, in milli-g.
 */
int Gyroscope::getY()
{
    requestUpdate();
    return sample.y;
}

/**
 * reads the value of the z axis from the latest update retrieved from the accelerometer,
 * usingthe default coordinate system as specified in the constructor.
 *
 * @return the force measured in the z axis, in milli-g.
 */
int Gyroscope::getZ()
{
    requestUpdate();
    return sample.z;
}

/**
  * Destructor for FXS8700, where we deregister from the array of fiber components.
  */
Gyroscope::~Gyroscope()
{
}

