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

#ifndef CODAL_STD_SPIFLASH_H
#define CODAL_STD_SPIFLASH_H

#include "SPIFlash.h"
#include "SPI.h"

namespace codal
{
class StandardSPIFlash : public SPIFlash
{
protected:
    uint32_t _numPages;
    SPI &spi;
    Pin &ssel;
    uint8_t cmdBuf[4];
    uint8_t status;

    void setCommand(uint8_t command, int addr);
    int sendCommand(uint8_t command, int addr = -1, void *resp = 0, int respSize = 0);
    int eraseCore(uint8_t cmd, uint32_t addr);
    int waitBusy(int waitMS);
    void writeEnable();

public:
    StandardSPIFlash(SPI &spi, Pin &ssel, int numPages);
    virtual int numPages();
    virtual int readBytes(uint32_t addr, void *buffer, uint32_t len);
    virtual int writeBytes(uint32_t addr, const void *buffer, uint32_t len);
    virtual int eraseSmallRow(uint32_t addr);
    virtual int eraseBigRow(uint32_t addr);
    virtual int eraseChip();
};
}

#endif
