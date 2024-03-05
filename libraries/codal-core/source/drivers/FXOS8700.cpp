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
 * Class definition for an FXS8700 3 axis accelerometer.
 *
 * Represents an implementation of the Freescale FXS8700 3 axis accelerometer
 * Also includes basic data caching and on demand activation.
 */
#include "FXOS8700.h"
#include "ErrorNo.h"
#include "Event.h"
#include "CodalCompat.h"
#include "CodalFiber.h"
#include "CodalDmesg.h"
#include "Accelerometer.h"

using namespace codal;

//
// Configuration table for available g force ranges.
// Maps g -> XYZ_DATA_CFG bit [0..1]
//
static const KeyValueTableEntry accelerometerRangeData[] = {
    {2,0},
    {4,1},
    {8,2}
};
CREATE_KEY_VALUE_TABLE(accelerometerRange, accelerometerRangeData);

//
// Configuration table for available data update frequency.
// maps microsecond period -> CTRL_REG1 data rate selection bits [3..5]
//
static const KeyValueTableEntry accelerometerPeriodData[] = {
    {2500,0x00},
    {5000,0x08},
    {10000,0x10},
    {20000,0x18},
    {80000,0x20},
    {160000,0x28},
    {320000,0x30},
    {1280000,0x38}
};
CREATE_KEY_VALUE_TABLE(accelerometerPeriod, accelerometerPeriodData);


/**
  * Configures the accelerometer for G range and sample rate defined
  * in this object. The nearest values are chosen to those defined
  * that are supported by the hardware. The instance variables are then
  * updated to reflect reality.
  *
  * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the accelerometer could not be configured.
  */
int FXOS8700::configure()
{
    int result;
    uint8_t value;

    // First find the nearest sample rate to that specified.
    Accelerometer::samplePeriod = accelerometerPeriod.getKey(Accelerometer::samplePeriod * 2000) / 1000;
    Accelerometer::sampleRange = accelerometerRange.getKey(Accelerometer::sampleRange);
    Compass::samplePeriod = Accelerometer::samplePeriod;

    // Now configure the accelerometer accordingly.

    // Firstly, disable the module (as some registers cannot be changed while its running).
    value = 0x00;
    result = i2c.writeRegister(address, FXOS8700_CTRL_REG1, value);
    if (result != 0)
    {
        DMESG("I2C ERROR: FXOS8700_CTRL_REG1");
        return DEVICE_I2C_ERROR;
    }

    // Enter hybrid mode (interleave accelerometer and magnetometer samples).
    // Also, select full oversampling on the magnetometer
    // TODO: Determine power / accuracy tradeoff here.
    value = 0x1F;
    result = i2c.writeRegister(address, FXOS8700_M_CTRL_REG1, value);
    if (result != 0)
    {
        DMESG("I2C ERROR: FXOS8700_M_CTRL_REG1");
        return DEVICE_I2C_ERROR;
    }

    // Select the auto incremement mode, which allows a contiguous I2C block
    // read of both acceleromter and magnetometer data despite them being non-contguous
    // in memory... funky!
    value = 0x20;
    result = i2c.writeRegister(address, FXOS8700_M_CTRL_REG2, value);
    if (result != 0)
    {
        DMESG("I2C ERROR: FXOS8700_M_CTRL_REG2");
        return DEVICE_I2C_ERROR;
    }

    // Configure Active LOW interrupt mode.
    // Use OpenDrain configuation if we're on a shared IRQ line, PUSHPULL configuration otherwise. 
#if CONFIG_ENABLED(DEVICE_I2C_IRQ_SHARED)
    value = 0x01;
#else
    value = 0x00;
#endif
    result = i2c.writeRegister(address, FXOS8700_CTRL_REG3, value);
    if (result != 0)
    {
        DMESG("I2C ERROR: FXOS8700_CTRL_REG3");
        return DEVICE_I2C_ERROR;
    }

    // Enable a data ready interrupt.
    value = 0x01;
    result = i2c.writeRegister(address, FXOS8700_CTRL_REG4, value);
    if (result != 0)
    {
        DMESG("I2C ERROR: FXOS8700_CTRL_REG4");
        return DEVICE_I2C_ERROR;
    }

    // Route the data ready interrupt to INT1 pin.
    value = 0x01;
    result = i2c.writeRegister(address, FXOS8700_CTRL_REG5, value);
    if (result != 0)
    {
        DMESG("I2C ERROR: FXOS8700_CTRL_REG5");
        return DEVICE_I2C_ERROR;
    }

    // Configure acceleromter g range.
    value = accelerometerRange.get(Accelerometer::sampleRange);
    result = i2c.writeRegister(address, FXOS8700_XYZ_DATA_CFG, value);
    if (result != 0)
    {
        DMESG("I2C ERROR: FXOS8700_XYZ_DATA_CFG");
        return DEVICE_I2C_ERROR;
    }

    // Configure sample rate and re-enable the sensor.
    value = accelerometerPeriod.get(Accelerometer::samplePeriod * 1000) | 0x01;
    result = i2c.writeRegister(address, FXOS8700_CTRL_REG1, value);
    if (result != 0)
    {
        DMESG("I2C ERROR: FXOS8700_CTRL_REG1");
        return DEVICE_I2C_ERROR;
    }

    return DEVICE_OK;
}

/**
  * Constructor.
  * Create a software abstraction of an FXSO8700 combined accelerometer/magnetometer
  *
  * @param _i2c an instance of I2C used to communicate with the device.
  *
  * @param address the default I2C address of the accelerometer. Defaults to: FXS8700_DEFAULT_ADDR.
  *
 */
FXOS8700::FXOS8700(I2C &_i2c, Pin &_int1, CoordinateSpace &coordinateSpace, uint16_t address, uint16_t aid, uint16_t cid) : Accelerometer(coordinateSpace, aid), Compass(coordinateSpace, cid), i2c(_i2c), int1(_int1)
{
    // Store our identifiers.
    this->address = address;

    // Configure and enable the accelerometer.
    configure();
}

/**
 * Attempts to read the 8 bit WHO_AM_I value from the accelerometer
 *
 * @return true if the WHO_AM_I value is succesfully read. false otherwise.
 */
int FXOS8700::isDetected(I2C &i2c, uint16_t address)
{
    return i2c.readRegister(address, FXOS8700_WHO_AM_I) == FXOS8700_WHOAMI_VAL;
}

/**
  * Reads the sensor ata from the FXSO8700, and stores it in our buffer.
  * This only happens if the device indicates that it has new data via int1.
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
int FXOS8700::requestUpdate()
{
    // Ensure we're scheduled to update the data periodically
    Accelerometer::status |= DEVICE_COMPONENT_STATUS_IDLE_TICK;

    // Poll interrupt line from device (ACTIVE LOW)
    if(int1.isActive())
    {
        uint8_t data[12];
        FXOSRawSample sample;
        uint8_t *lsb = (uint8_t *) &sample;
        uint8_t *msb = lsb + 1;
        int result;

#if CONFIG_ENABLED(DEVICE_I2C_IRQ_SHARED)
        // Determine if this device has all its data ready (we may be on a shared IRQ line)
        uint8_t status_reg = i2c.readRegister(address, FXOS8700_STATUS_REG);
        if((status_reg & FXOS8700_STATUS_DATA_READY) != FXOS8700_STATUS_DATA_READY)
            return DEVICE_OK;
#endif

        // Read the combined accelerometer and magnetometer data.
        result = i2c.readRegister(address, FXOS8700_OUT_X_MSB, data, 12);

        if (result !=0)
            return DEVICE_I2C_ERROR;
        
        // read sensor data (and translate into signed little endian)
        for (int i=0; i<12; i+=2)
        {
            *msb = data[i]; 
            *lsb = data[i+1];
            msb += 2;
            lsb += 2;
        }

        // scale the 14 bit accelerometer data (packed into 16 bits) into SI units (milli-g), and translate to ENU coordinate system
        Accelerometer::sampleENU.x = (-sample.ay * Accelerometer::sampleRange) / 32;
        Accelerometer::sampleENU.y = (sample.ax * Accelerometer::sampleRange) / 32;
        Accelerometer::sampleENU.z = (sample.az * Accelerometer::sampleRange) / 32;

        // translate magnetometer data into ENU coordinate system and normalise into nano-teslas
        Compass::sampleENU.x = FXOS8700_NORMALIZE_SAMPLE(-sample.cy);
        Compass::sampleENU.y = FXOS8700_NORMALIZE_SAMPLE(sample.cx);
        Compass::sampleENU.z = FXOS8700_NORMALIZE_SAMPLE(sample.cz);

        Accelerometer::update();
        Compass::update();    
    }

    return DEVICE_OK;
}

/**
  * A periodic callback invoked by the fiber scheduler idle thread.
  *
  * Internally calls updateSample().
  */
void FXOS8700::idleCallback()
{
    requestUpdate();
}

/**
  * Destructor for FXS8700, where we deregister from the array of fiber components.
  */
FXOS8700::~FXOS8700()
{
}

