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

#include "I2C.h"
#include "ErrorNo.h"

namespace codal
{
    /**
     * Constructor.
     */
    I2C::I2C(Pin &sda, Pin &scl)
    {
    }

    /**
     * Set the frequency of the I2C interface
     *
     * @param frequency The bus frequency in hertz
     */
    int I2C::setFrequency(uint32_t frequency)
    {
        return DEVICE_NOT_IMPLEMENTED;
    }

    /**
     * Issues a START condition on the I2C bus
     */
    int I2C::start()
    {
        return DEVICE_NOT_IMPLEMENTED;
    }

    /**
     * Issues a STOP condition on the I2C bus
     */
    int I2C::stop()
    {
        return DEVICE_NOT_IMPLEMENTED;
    }

    /**
    * Writes the given byte to the I2C bus.
    *
    * The CPU will busy wait until the transmission is complete.
    *
    * @param data The byte to write.
    * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the write request failed.
    */
    int I2C::write(uint8_t data)
    {
        return DEVICE_NOT_IMPLEMENTED;
    }

    /**
    * Reads a single byte from the I2C bus.
    * The CPU will busy wait until the transmission is complete.
    *
    * @return the byte read from the I2C bus, or DEVICE_I2C_ERROR if the the write request failed.
    */
    int I2C::read(AcknowledgeType ack)
    {
        return DEVICE_NOT_IMPLEMENTED;
    }

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
    int I2C::write(uint16_t address, uint8_t data)
    {
        return write(address, &data, 1);
    }

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
     *
     * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the write request failed.
     */
    int I2C::write(uint16_t address, uint8_t *data, int len, bool repeated)
    {
        if (data == NULL || len <= 0)
            return DEVICE_INVALID_PARAMETER; // Send a start condition

        start();

        // Send the address of the slave, with a write bit set.
        write((uint8_t)address);

        // Send the body of the data
        for (int i = 0; i < len; i++)
            write(data[i]);

        // Send a stop condition
        if (!repeated)
            stop();

        return DEVICE_OK;
    }

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
    int I2C::writeRegister(uint16_t address, uint8_t reg, uint8_t value)
    {
        uint8_t command[2];
        command[0] = reg;
        command[1] = value;

        return write(address, command, 2);
    }

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
    *
    * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the read request failed.
    */
    int I2C::read(uint16_t address, uint8_t *data, int len, bool repeated)
    {
        int i = 0;

        if (data == NULL || len <= 0)
            return DEVICE_INVALID_PARAMETER;

        // Send a start condition
        start();

        // Send the address of the slave, with a read bit set.
        write((uint8_t)(address | 0x01));

        // Read the body of the data
        for (i = 0; i < len-1; i++)
            data[i] = read();

        data[i] = read(NACK);

        // Send a stop condition
        if (!repeated)
            stop();

        return DEVICE_OK;
    }

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
    int I2C::readRegister(uint16_t address, uint8_t reg, uint8_t *data, int length, bool repeated)
    {
        int result;

        if (repeated)
            result = write(address, &reg, 1, true);
        else
            result = write(address, reg);

        if (result != DEVICE_OK)
            return result;

        result = read(address, data, length);
        if (result != DEVICE_OK)
            return result;

        return DEVICE_OK;
    }

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
    int I2C::readRegister(uint8_t address, uint8_t reg)
    {
        int result;
        uint8_t data;

        result = readRegister(address, reg, &data, 1);

        return (result == DEVICE_OK) ? (int)data : result;
    }

    int I2C::write(int address, char *data, int len, bool repeated)
    {
        return write((uint16_t)address, (uint8_t *)data, len, repeated);
    }

    int I2C::read(int address, char *data, int len, bool repeated)
    {
        return read((uint16_t)address, (uint8_t *)data, len, repeated);
    }
}

