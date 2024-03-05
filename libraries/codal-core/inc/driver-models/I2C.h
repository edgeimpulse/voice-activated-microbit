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

#ifndef CODAL_I2C_H
#define CODAL_I2C_H

#include "CodalConfig.h"
#include "ErrorNo.h"
#include "Pin.h"

namespace codal
{
/**
  * Class definition for an I2C interface.
  */

enum AcknowledgeType {ACK, NACK};

class I2C
{
public:
    I2C(Pin &sda, Pin &scl);

    /** Set the frequency of the I2C interface
      *
      * @param frequency The bus frequency in hertz
      */
    virtual int setFrequency(uint32_t frequency);

protected:
    /**
     * Issues a START condition on the I2C bus
     * @return DEVICE_OK on success, or an error code
     */
    virtual int start();

    /**
     * Issues a STOP condition on the I2C bus
     * @return DEVICE_OK on success, or an error code
     */
    virtual int stop();

    /**
      * Writes the given byte to the I2C bus.
      *
      * The CPU will busy wait until the transmission is complete.
      *
      * @param data The byte to write.
      * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the write request failed.
      */
    virtual int write(uint8_t data);

    /**
    * Reads a single byte from the I2C bus.
    * The CPU will busy wait until the transmission is complete.
    *
    * @return the byte read from the I2C bus, or DEVICE_I2C_ERROR if the the write request failed.
    */
    virtual int read(AcknowledgeType ack = ACK);

public:
    /**
      * Issues a standard, 2 byte I2C command write to the I2C bus.
      * This consists of:
      *  - Asserting a Start condition on the bus
      *  - Selecting the Slave address (as an 8 bit address)
      *  - Writing the raw 8 bit data provided
      *  - Asserting a Stop condition on the bus
      *
      * The CPU will busy wait until the transmission is complete.
      *
      * @param address The 8bit I2C address of the device to write to
      * @param data the byte command to write
      *
      * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the write request failed.
      */
    virtual int write(uint16_t address, uint8_t data);

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
    int write(int address, char *data, int len, bool repeated = false);

    /**
      * Performs a typical register write operation to the I2C slave device provided.
      * This consists of:
      *  - Asserting a Start condition on the bus
      *  - Selecting the Slave address (as an 8 bit address)
      *  - Writing the 8 bit register address provided
      *  - Writing the 8 bit value provided
      *  - Asserting a Stop condition on the bus
      *
      * The CPU will busy wait until the transmission is complete..
      *
      * @param address 8bit address of the device to write to
      * @param reg The 8bit address of the register to write to.
      * @param value The value to write.
      *
      * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the write request failed.
      */
    virtual int writeRegister(uint16_t address, uint8_t reg, uint8_t value);

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
    int read(int address, char *data, int len, bool repeated = false);

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
     * Issues a single byte read command, and returns the value read, or an error.
     *
     * Blocks the calling thread until complete.
     *
     * @param address The address of the I2C device to write to.
     * @param reg The address of the register to access.
     *
     * @return the byte read on success, DEVICE_INVALID_PARAMETER or DEVICE_I2C_ERROR if the the read request failed.
     */
    virtual int readRegister(uint8_t address, uint8_t reg);
};
}

#endif
