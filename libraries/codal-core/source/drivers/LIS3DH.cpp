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
 * Class definition for an LIS3DH 3 axis accelerometer.
 *
 * Represents an implementation of the Freescale LIS3DH 3 axis accelerometer
 * Also includes basic data caching and on demand activation.
 */
#include "CodalConfig.h"
#include "LIS3DH.h"
#include "ErrorNo.h"
#include "CodalCompat.h"
#include "CodalFiber.h"

using namespace codal;

//
// Configuration table for available g force ranges.
// Maps g -> LIS3DH_CTRL_REG4 [5..4]
//
static const KeyValueTableEntry accelerometerRangeData[] = {
    {2, 0},
    {4, 1},
    {8, 2},
    {16, 3}
};
CREATE_KEY_VALUE_TABLE(accelerometerRange, accelerometerRangeData);

//
// Configuration table for available data update frequency.
// maps microsecond period -> LIS3DH_CTRL_REG1 data rate selection bits
//
static const KeyValueTableEntry accelerometerPeriodData[] = {
    {2500,      0x70},
    {5000,      0x60},
    {10000,     0x50},
    {20000,     0x40},
    {40000,     0x30},
    {100000,    0x20},
    {1000000,   0x10}
};
CREATE_KEY_VALUE_TABLE(accelerometerPeriod, accelerometerPeriodData);

/**
  * Constructor.
  * Create a software abstraction of an accelerometer.
  *
  * @param _i2c an instance of DeviceI2C used to communicate with the onboard accelerometer.
  * @param _int1 the pin connected to the LIS3DH IRQ line.
  * @param coordinateSpace The orientation of the sensor.
  * @param address the default I2C address of the accelerometer. Defaults to: LIS3DH_DEFAULT_ADDR.
  * @param id the unique EventModel id of this component. Defaults to: DEVICE_ID_ACCELEROMETER
  *
  * @code
  * DeviceI2C i2c = DeviceI2C(I2C_SDA0, I2C_SCL0);
  *
  * LIS3DH accelerometer = LIS3DH(i2c);
  * @endcode
 */
LIS3DH::LIS3DH(I2C& _i2c, Pin &_int1, CoordinateSpace &coordinateSpace, uint16_t address,  uint16_t id) : Accelerometer(coordinateSpace, id), i2c(_i2c), int1(_int1)
{
    // Store our identifiers.
    this->id = id;
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
  */
int LIS3DH::configure()
{
    int result;
    uint8_t value;

    // First find the nearest sample rate to that specified.
    samplePeriod = accelerometerPeriod.getKey(samplePeriod * 1000) / 1000;
    sampleRange = accelerometerRange.getKey(sampleRange);

    // Now configure the accelerometer accordingly.
    // Firstly, Configure for normal precision operation at the sample rate requested.
    value = accelerometerPeriod.get(samplePeriod * 1000) | 0x07;
    result = i2c.writeRegister(address, LIS3DH_CTRL_REG1, value);
    if (result != 0)
        return DEVICE_I2C_ERROR;

    // Enable the INT1 interrupt pin when XYZ data is available.
    value = 0x10;
    result = i2c.writeRegister(address, LIS3DH_CTRL_REG3, value);
    if (result != 0)
        return DEVICE_I2C_ERROR;

    // Configure for the selected g range.
    value = accelerometerRange.get(sampleRange) << 4;
    result = i2c.writeRegister(address, LIS3DH_CTRL_REG4,  value);
    if (result != 0)
        return DEVICE_I2C_ERROR;

    // Configure for a latched interrupt request.
    value = 0x08;
    result = i2c.writeRegister(address, LIS3DH_CTRL_REG5, value);
    if (result != 0)
        return DEVICE_I2C_ERROR;

    return DEVICE_OK;
}


/**
  * Attempts to read the 8 bit ID from the accelerometer, this can be used for
  * validation purposes.
  *
  * @return the 8 bit ID returned by the accelerometer, or DEVICE_I2C_ERROR if the request fails.
  *
  * @code
  * accelerometer.whoAmI();
  * @endcode
  */
int LIS3DH::whoAmI()
{
    uint8_t data;
    int result;

    result = i2c.readRegister(address, LIS3DH_WHOAMI, &data, 1);
    if (result !=0)
        return DEVICE_I2C_ERROR;

    return (int)data;
}

/**
  * Reads the acceleration data from the accelerometer, and stores it in our buffer.
  * This only happens if the accelerometer indicates that it has new data via int1.
  *
  * On first use, this member function will attempt to add this component to the
  * list of fiber components in order to constantly update the values stored
  * by this object.
  *
  * This technique is called lazy instantiation, and it means that we do not
  * obtain the overhead from non-chalantly adding this component to fiber components.
  *
  * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the read request fails.
  */
int LIS3DH::requestUpdate()
{
    // Ensure we're scheduled to update the data periodically
    status |= DEVICE_COMPONENT_STATUS_IDLE_TICK;

    // Poll interrupt line from accelerometer.
    if(int1.getDigitalValue() == 1)
    {
        int8_t data[6];
        uint8_t src;
        int result;

        // read the XYZ data (16 bit)
        // n.b. we need to set the MSB bit to enable multibyte transfers from this device (WHY? Who Knows!)
        result = i2c.readRegister(address, 0x80 | LIS3DH_OUT_X_L, (uint8_t *)data, 6);

        if (result !=0)
            return DEVICE_I2C_ERROR;

        target_wait_us(3);

        // Acknowledge the interrupt.
        i2c.readRegister(address, LIS3DH_INT1_SRC, &src, 1);

        // read MSB values...
        sampleENU.x = data[1];
        sampleENU.y = data[3];
        sampleENU.z = data[5];

        // Normalize the data in the 0..1024 range.
        sampleENU.x *= 8;
        sampleENU.y *= 8;
        sampleENU.z *= 8;

#if CONFIG_ENABLED(USE_ACCEL_LSB)
        // Add in LSB values.
        sampleENU.x += (data[0] / 64);
        sampleENU.y += (data[2] / 64);
        sampleENU.z += (data[4] / 64);
#endif

        // Scale into millig (approx!). (LIS3DH is ENU aligned)
        sampleENU.x *= this->sampleRange;
        sampleENU.y *= this->sampleRange;
        sampleENU.z *= this->sampleRange;
 
        // Indicate that a new sample is available
        update();
    }

    return DEVICE_OK;
};


/**
  * A periodic callback invoked by the fiber scheduler idle thread.
  *
  * Internally calls updateSample().
  */
void LIS3DH::idleCallback()
{
    requestUpdate();
}

/**
  * Destructor for LIS3DH, where we deregister from the array of fiber components.
  */
LIS3DH::~LIS3DH()
{
}

int LIS3DH::setSleep(bool sleepMode)
{
    if (sleepMode)
        return i2c.writeRegister(this->address, LIS3DH_CTRL_REG1, 0x00);
    else
        return configure();
}