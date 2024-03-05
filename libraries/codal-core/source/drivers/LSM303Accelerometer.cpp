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
 * Class definition for an LSM303 3 axis accelerometer.
 *
 * Represents an implementation of the LSM303 3 axis accelerometer
 * Also includes basic data caching and on demand activation.
 */
#include "CodalConfig.h"
#include "CodalDmesg.h"
#include "LSM303Accelerometer.h"
#include "ErrorNo.h"
#include "Event.h"
#include "CodalCompat.h"
#include "CodalFiber.h"

using namespace codal;

//
// Configuration table for available g force ranges.
// Maps g ->  CTRL_REG4 full scale selection bits [4..5]
//
static const KeyValueTableEntry accelerometerRangeData[] = {
    {2, 0x00},
    {4, 0x10},
    {8, 0x20},
    {16, 0x30}
};
CREATE_KEY_VALUE_TABLE(accelerometerRange, accelerometerRangeData);

//
// Configuration table for available data update frequency.
// maps microsecond period -> CTRL_REG1 data rate selection bits [4..7]
//
static const KeyValueTableEntry accelerometerPeriodData[] = {
    {617, 0x80},
    {744, 0x90},
    {2500, 0x70},
    {5000, 0x60},
    {10000, 0x50},
    {20000, 0x40},
    {40000, 0x30},
    {100000, 0x20},
    {1000000, 0x10}
};
CREATE_KEY_VALUE_TABLE(accelerometerPeriod, accelerometerPeriodData);


/**
 * Constructor.
 * Create a software abstraction of an accelerometer.
 *
 * @param coordinateSpace The orientation of the sensor. Defaults to: SIMPLE_CARTESIAN
 * @param id The unique EventModel id of this component. Defaults to: DEVICE_ID_ACCELEROMETER
 *
 */
LSM303Accelerometer::LSM303Accelerometer(I2C& _i2c, Pin &_int1, CoordinateSpace &coordinateSpace, uint16_t address, uint16_t id) : Accelerometer(coordinateSpace, id), i2c(_i2c), int1(_int1)
{
    // Store our identifiers.
    this->status = 0;
    this->address = address;

    // Configure and enable the accelerometer.
    configure();
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
int LSM303Accelerometer::configure()
{
    int result;

    // First find the nearest sample rate to that specified.
    samplePeriod = accelerometerPeriod.getKey(samplePeriod * 1000) / 1000;
    sampleRange = accelerometerRange.getKey(sampleRange);

    // Now configure the accelerometer accordingly.

    // Place the device into normal (10 bit) mode, with all axes enabled at the nearest supported data rate to that  requested.
    result = i2c.writeRegister(address, LSM303_CTRL_REG1_A, status & LSM303_A_STATUS_ENABLED ? accelerometerPeriod.get(samplePeriod * 1000) | 0x07 : 0x00);
    if (result != 0)
    {
        DMESG("LSM303 INIT: ERROR WRITING LSM303_CTRL_REG1_A");
        return DEVICE_I2C_ERROR;
    }

    // Enable the DRDY1 interrupt on INT1 pin.
    result = i2c.writeRegister(address, LSM303_CTRL_REG3_A, 0x10);
    if (result != 0)
    {
        DMESG("LSM303 INIT: ERROR WRITING LSM303_CTRL_REG3_A");
        return DEVICE_I2C_ERROR;
    }

    // Select the g range to that requested, using little endian data format and disable self-test and high rate functions.
    result = i2c.writeRegister(address, LSM303_CTRL_REG4_A, 0x80 | accelerometerRange.get(sampleRange));
    if (result != 0)
    {
        DMESG("LSM303 INIT: ERROR WRITING LSM303_CTRL_REG4_A");
        return DEVICE_I2C_ERROR;
    }

    return DEVICE_OK;
}

/**
 * Poll to see if new data is available from the hardware. If so, update it.
 * n.b. it is not necessary to explicitly call this funciton to update data
 * (it normally happens in the background when the scheduler is idle), but a check is performed
 * if the user explicitly requests up to date data.
 *
 * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the update fails.
 *
 * @note This method should be overidden by the hardware driver to implement the requested
 * changes in hardware.
 */
int LSM303Accelerometer::requestUpdate()
{
    bool awaitSample = false;

    if ((status & (LSM303_A_STATUS_ENABLED | LSM303_A_STATUS_SLEEPING)) == 0x00)
    {
        // If we get here without being enabled, applicaiton code has requested
        // functionlity from this component. Perform on demand activation.
        status |= LSM303_A_STATUS_ENABLED;
        status |= DEVICE_COMPONENT_STATUS_IDLE_TICK;
        configure();

        // Ensure the first sample is accurate.
        awaitSample = true;
    }    

    // Poll interrupt line from device
    do
    {
        if(int1.isActive())
        {
            uint8_t data[6];
            int result;
            int16_t *x;
            int16_t *y;
            int16_t *z;

    #if CONFIG_ENABLED(DEVICE_I2C_IRQ_SHARED)
            // Determine if this device has all its data ready (we may be on a shared IRQ line)
            uint8_t status_reg = i2c.readRegister(address, LSM303_STATUS_REG_A);
            if((status_reg & LSM303_A_STATUS_DATA_READY) != LSM303_A_STATUS_DATA_READY)
            {
                if (awaitSample)
                    continue;
                else
                    return DEVICE_OK;
            }
    #endif

            // Read the combined accelerometer and magnetometer data.
            result = i2c.readRegister(address, LSM303_OUT_X_L_A | 0x80, data, 6);
            awaitSample = false;

            if (result !=0)
                return DEVICE_I2C_ERROR;

            // Read in each reading as a 16 bit little endian value, and scale to 10 bits.
            x = ((int16_t *) &data[0]);
            y = ((int16_t *) &data[2]);
            z = ((int16_t *) &data[4]);

            *x = *x / 32;
            *y = *y / 32;
            *z = *z / 32;

            // Scale into millig (approx) and align to ENU coordinate system
            sampleENU.x = -((int)(*y)) * sampleRange;
            sampleENU.y = -((int)(*x)) * sampleRange;
            sampleENU.z =  ((int)(*z)) * sampleRange;

            // indicate that new data is available.
            update();
        }
    } while (awaitSample);

    return DEVICE_OK;
}

/**
  * A periodic callback invoked by the fiber scheduler idle thread.
  *
  * Internally calls updateSample().
  */
void LSM303Accelerometer::idleCallback()
{
    requestUpdate();
}

/**
 * Puts the component in (or out of) sleep (low power) mode.
 */
int LSM303Accelerometer::setSleep(bool doSleep)
{
    if (doSleep && (status & LSM303_A_STATUS_ENABLED))
    {
        status |= LSM303_A_STATUS_SLEEPING;
        status &= ~LSM303_A_STATUS_ENABLED;
        configure();
    }
    
    if (!doSleep && (status & LSM303_A_STATUS_SLEEPING))
    {
        status |= LSM303_A_STATUS_ENABLED;
        status &= ~LSM303_A_STATUS_SLEEPING;
        configure();
    }
   
    return DEVICE_OK;
}


/**
 * Attempts to read the 8 bit WHO_AM_I value from the accelerometer
 *
 * @return true if the WHO_AM_I value is succesfully read. false otherwise.
 */
int LSM303Accelerometer::isDetected(I2C &i2c, uint16_t address)
{
    return i2c.readRegister(address, LSM303_WHO_AM_I_A) == LSM303_A_WHOAMI_VAL;
}

/**
 * Destructor.
 */
LSM303Accelerometer::~LSM303Accelerometer()
{
}

