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

#ifndef DEVICE_ST7735_H
#define DEVICE_ST7735_H

#include "Pin.h"
#include "SPI.h"
#include "Event.h"
#include "ScreenIO.h"

namespace codal
{

struct ST7735WorkBuffer;

#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20
#define MADCTL_ML 0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH 0x04

class ST7735 : public CodalComponent
{
protected:
    ScreenIO &io;
    Pin *cs;
    Pin *dc;
    uint8_t cmdBuf[20];
    ST7735WorkBuffer *work;
    bool inSleepMode;

    // if true, every pixel will be plotted as 4 pixels and 16 bit color mode
    // will be used; this is for ILI9341 which usually has 320x240 screens
    // and doesn't support 12 bit color
    bool double16;

    void beginCS() { if (cs) cs->setDigitalValue(0); }
    void endCS() { if (cs) cs->setDigitalValue(1); }
    void setCommand() { dc->setDigitalValue(0); }
    void setData() { dc->setDigitalValue(1); }

    void sendCmd(uint8_t *buf, int len);
    void sendCmdSeq(const uint8_t *buf);
    void sendDone(Event);
    void sendWords(unsigned numBytes);
    void startTransfer(unsigned size);
    void sendBytes(unsigned num);
    void startRAMWR(int cmd = 0);

    static void sendColorsStep(ST7735 *st);

public:
    ST7735(ScreenIO &io, Pin &cs, Pin &dc);
    virtual int init();

    /**
     * Configure screen-specific parameters.
     *
     * @param madctl See MADCTL_* constants above
     * @param frmctr1 defaults to 0x083b3b, 0x053a3a, 0x053c3c depending on screen size; 0x000605
     * was found to work well on 160x128 screen; big-endian
     */
    void configure(uint8_t madctl, uint32_t frmctr1);
    /**
     * Set rectangle where pixels sent by sendIndexedImage() will be stored.
     */
    void setAddrWindow(int x, int y, int w, int h);
    /**
     * Send 4 bit indexed color image, little endian, column-major, using specified palette (use
     * NULL if unchanged).
     */
    int sendIndexedImage(const uint8_t *src, unsigned width, unsigned height, uint32_t *palette);
    /**
     * Waits for the previous sendIndexedImage() operation to complete (it normally executes in
     * background).
     */
    void waitForSendDone();

    /**
     * Puts the display in (or out of) sleep mode.
     */
    virtual int setSleep(bool sleepMode);
};

} // namespace codal

#endif