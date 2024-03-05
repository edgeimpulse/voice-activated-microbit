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

#include "StandardSPIFlash.h"

using namespace codal;

//#define check(cond) if(!(cond)) return DEVICE_INVALID_PARAMETER
#define check(cond)                                                                                \
    if (!(cond))                                                                                   \
    target_panic(909)

StandardSPIFlash::StandardSPIFlash(SPI &spi, Pin &ssel, int numPages)
    : _numPages(numPages), spi(spi), ssel(ssel)
{
    ssel.setDigitalValue(1);
}

void StandardSPIFlash::setCommand(uint8_t command, int addr)
{
    cmdBuf[0] = command;
    cmdBuf[1] = addr >> 16;
    cmdBuf[2] = addr >> 8;
    cmdBuf[3] = addr >> 0;
}

int StandardSPIFlash::sendCommand(uint8_t command, int addr, void *resp, int respSize)
{
    setCommand(command, addr);

    ssel.setDigitalValue(0);
    int r = spi.transfer(cmdBuf, addr == -1 ? 1 : 4, NULL, 0);
    if (r == DEVICE_OK)
        r = spi.transfer(NULL, 0, (uint8_t *)resp, respSize);
    ssel.setDigitalValue(1);

    return r;
}

void StandardSPIFlash::writeEnable()
{
    sendCommand(0x06);
}

int StandardSPIFlash::waitBusy(int waitMS)
{
    do
    {
        int r = sendCommand(0x05, -1, &status, 1);
        if (r < 0)
            return r;
        if (waitMS)
            fiber_sleep(waitMS);
    } while (status & 0x01);

    return DEVICE_OK;
}

int StandardSPIFlash::numPages()
{
    return _numPages;
}

int StandardSPIFlash::readBytes(uint32_t addr, void *buffer, uint32_t len)
{
    check(addr + len <= _numPages * SPIFLASH_PAGE_SIZE);
    check(addr <= _numPages * SPIFLASH_PAGE_SIZE);
    return sendCommand(0x03, addr, buffer, len);
}

int StandardSPIFlash::writeBytes(uint32_t addr, const void *buffer, uint32_t len)
{
    check(len <= SPIFLASH_PAGE_SIZE);
    check(addr / SPIFLASH_PAGE_SIZE == (addr + len - 1) / SPIFLASH_PAGE_SIZE);
    check(addr + len <= _numPages * SPIFLASH_PAGE_SIZE);

    writeEnable();

    setCommand(0x02, addr);

    ssel.setDigitalValue(0);
    for (uint32_t i = 0; i < 4; ++i)
        if (spi.write(cmdBuf[i]) < 0)
            goto fail;
    for (uint32_t i = 0; i < len; ++i)
        if (spi.write(((const uint8_t *)buffer)[i]) < 0)
            goto fail;
    ssel.setDigitalValue(1);

    // the typical write time is under 1ms, so we don't bother with fiber_sleep()
    return waitBusy(0);

fail:
    ssel.setDigitalValue(1);
    return DEVICE_SPI_ERROR;
}

int StandardSPIFlash::eraseCore(uint8_t cmd, uint32_t addr)
{
    writeEnable();
    int r = sendCommand(cmd, addr);
    if (r < 0)
        return r;
    return waitBusy(10);
}

int StandardSPIFlash::eraseSmallRow(uint32_t addr)
{
    check(addr < _numPages * SPIFLASH_PAGE_SIZE);
    check((addr & (SPIFLASH_SMALL_ROW_SIZE - 1)) == 0);
    return eraseCore(0x20, addr);
}

int StandardSPIFlash::eraseBigRow(uint32_t addr)
{
    check(addr < _numPages * SPIFLASH_PAGE_SIZE);
    check((addr & (SPIFLASH_BIG_ROW_SIZE - 1)) == 0);
    return eraseCore(0xD8, addr);
}

int StandardSPIFlash::eraseChip()
{
    return eraseCore(0xC7, -1); // or 0x60
}
