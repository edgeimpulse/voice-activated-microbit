/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

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
#ifndef CODAL_NRF52I2C_H
#define CODAL_NRF52I2C_H

#include "CodalConfig.h"
#include "codal-core/inc/driver-models/I2C.h"
#include "NRF52Pin.h"
#include "hal/nrf_twim.h"

namespace codal
{
/**
 * Class definition for I2C service
 */
class NRF52I2C : public codal::I2C
{
    int minimumBusIdlePeriod;

    int waitForStop(int evt);
protected:
    NRF52Pin &sda, &scl;
    NRF_TWIM_Type *p_twim;
public:
    /**
     * Constructor.
     */
    NRF52I2C(NRF52Pin &sda, NRF52Pin &scl, NRF_TWIM_Type *device = NULL);

    /** Set the frequency of the I2C interface
     *
     * @param frequency The bus frequency in hertz
     */
    virtual int setFrequency(uint32_t frequency);

    /**
    * Issues a standard, I2C command write to the I2C bus.
    * This consists of:
    *  - Asserting a Start condition on the bus
    *  - Selecting the Slave address (as an 8 bit address)
    *  - Writing a number of raw data bytes provided
    *  - Asserting a Stop condition on the bus
    *
    * The CPU will busy wait until the transmission is complete.
    *
    * @param address The 8bit I2C address of the device to write to
    * @param data pointer to the bytes to write
    * @param len the number of bytes to write
    * @param repeated Suppresses the generation of a STOP condition if set. Default: false;
    *
    * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the write request failed.
    */
    virtual int write(uint16_t address, uint8_t *data, int len, bool repeated = false);

    /**
      * Issues a standard, 2 byte I2C command read to the I2C bus.
      * This consists of:
      *  - Asserting a Start condition on the bus
      *  - Selecting the Slave address (as an 8 bit address)
      *  - reading "len" bytes of raw 8 bit data into the buffer provided
      *  - Asserting a Stop condition on the bus
      *
      * The CPU will busy wait until the transmission is complete.
      *
      * @param address The 8bit I2C address of the device to read from
      * @param data pointer to store the the bytes read
      * @param len the number of bytes to read into the buffer
      * @param repeated Suppresses the generation of a STOP condition if set. Default: false;
      *
      * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the read request failed.
      */
    virtual int read(uint16_t address, uint8_t *data, int len, bool repeated = false);

    /**
      * Performs a typical register read operation to the I2C slave device provided.
      * This consists of:
      *  - Asserting a Start condition on the bus
      *  - Selecting the Slave address (as an 8 bit address, I2C WRITE)
      *  - Selecting a RAM register address in the slave
      *  - Asserting a Stop condition on the bus
      *  - Asserting a Start condition on the bus
      *  - Selecting the Slave address (as an 8 bit address, I2C READ)
      *  - Performing an 8 bit read operation (of the requested register)
      *  - Asserting a Stop condition on the bus
      *
      * The CPU will busy wait until the transmission is complete..
      *
      * @param address 8bit I2C address of the device to read from
      * @param reg The 8bit register address of the to read.
      * @param data A pointer to a memory location to store the result of the read operation
      * @param length The number of mytes to read
      * @param repeated Use a repeated START/START/STOP transaction if true, or independent START/STOP/START/STOP transactions if fasle. Default: true
      *
      * @return DEVICE_OK or DEVICE_I2C_ERROR if the the read request failed.
      */
    virtual int readRegister(uint16_t address, uint8_t reg, uint8_t *data, int length, bool repeated = true);

    /**
      * Clear I2C bus
      */ 
    void clearBus();

    /**
     * Define the minimum bus idle period for this I2C bus.
     * Thise controls the period of time the bus will remain idle between I2C transactions, 
     * and also between subsequent write/read operations within a repeated START condition.
     *
     * @param period The minimum bus idle period, in microseconds
     * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER
     */
    int setBusIdlePeriod(int period);
};
} // namespace codal

#endif
