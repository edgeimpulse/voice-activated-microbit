#include "ST7735.h"
#include "CodalFiber.h"
#include "CodalDmesg.h"

#define SWAP 0

#define assert(cond)                                                                               \
    if (!(cond))                                                                                   \
    target_panic(909)

#define ST7735_NOP 0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID 0x04
#define ST7735_RDDST 0x09

#define ST7735_SLPIN 0x10
#define ST7735_SLPOUT 0x11
#define ST7735_PTLON 0x12
#define ST7735_NORON 0x13

#define ST7735_INVOFF 0x20
#define ST7735_INVON 0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E

#define ST7735_PTLAR 0x30
#define ST7735_COLMOD 0x3A
#define ST7735_MADCTL 0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_RDID1 0xDA
#define ST7735_RDID2 0xDB
#define ST7735_RDID3 0xDC
#define ST7735_RDID4 0xDD

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20
#define MADCTL_ML 0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH 0x04

namespace codal
{

ST7735::ST7735(ScreenIO &io, Pin &cs, Pin &dc) : io(io), cs(&cs), dc(&dc), work(NULL)
{
    double16 = false;
    inSleepMode = false;
}

#define DELAY 0x80

// clang-format off
static const uint8_t initCmds[] = {
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      120,                    //     150 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      120,                    //     500 ms delay
      #if 0
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x02, 0x2c, 0x2d,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
      #endif
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x03,                  //     12-bit color

    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      10,
    0, 0 // END
};
// clang-format on

// Nordic cannot send more than 255 bytes at a time;
// 224 aligns with a word
#ifdef NRF52_SERIES
#define DATABUFSIZE 224
#else
#define DATABUFSIZE 500
#endif

struct ST7735WorkBuffer
{
    unsigned width;
    unsigned height;
    uint8_t dataBuf[DATABUFSIZE];
    const uint8_t *srcPtr;
    unsigned x;
    uint32_t *paletteTable;
    unsigned srcLeft;
    bool inProgress;
    uint32_t expPalette[256];
};

void ST7735::sendBytes(unsigned num)
{
    assert(num > 0);
    if (num > work->srcLeft)
        num = work->srcLeft;
    work->srcLeft -= num;

    if (double16)
    {
        uint32_t *dst = (uint32_t *)work->dataBuf;
        while (num--)
        {
            uint8_t v = *work->srcPtr++;
            *dst++ = work->expPalette[v & 0xf];
            *dst++ = work->expPalette[v >> 4];
        }
        startTransfer((uint8_t *)dst - work->dataBuf);
    }
    else
    {
        uint8_t *dst = work->dataBuf;
        while (num--)
        {
            uint32_t v = work->expPalette[*work->srcPtr++];
            *dst++ = v;
            *dst++ = v >> 8;
            *dst++ = v >> 16;
        }
        startTransfer(dst - work->dataBuf);
    }
}

void ST7735::sendWords(unsigned numBytes)
{
    if (numBytes > work->srcLeft)
        numBytes = work->srcLeft & ~3;
    assert(numBytes > 0);
    work->srcLeft -= numBytes;
    uint32_t numWords = numBytes >> 2;
    const uint32_t *src = (const uint32_t *)work->srcPtr;
    uint32_t *tbl = work->expPalette;
    uint32_t *dst = (uint32_t *)work->dataBuf;

    if (double16)
        while (numWords--)
        {
            uint32_t v = *src++;
            *dst++ = tbl[0xf & (v >> 0)];
            *dst++ = tbl[0xf & (v >> 4)];
            *dst++ = tbl[0xf & (v >> 8)];
            *dst++ = tbl[0xf & (v >> 12)];
            *dst++ = tbl[0xf & (v >> 16)];
            *dst++ = tbl[0xf & (v >> 20)];
            *dst++ = tbl[0xf & (v >> 24)];
            *dst++ = tbl[0xf & (v >> 28)];
        }
    else
        while (numWords--)
        {
            uint32_t s = *src++;
            uint32_t o = tbl[s & 0xff];
            uint32_t v = tbl[(s >> 8) & 0xff];
            *dst++ = o | (v << 24);
            o = tbl[(s >> 16) & 0xff];
            *dst++ = (v >> 8) | (o << 16);
            v = tbl[s >> 24];
            *dst++ = (o >> 16) | (v << 8);
        }

    work->srcPtr = (uint8_t *)src;
    startTransfer((uint8_t *)dst - work->dataBuf);
}

void ST7735::sendColorsStep(ST7735 *st)
{
    ST7735WorkBuffer *work = st->work;

    if (work->paletteTable)
    {
        auto palette = work->paletteTable;
        work->paletteTable = NULL;
        memset(work->dataBuf, 0, sizeof(work->dataBuf));
        uint8_t *base = work->dataBuf;
        for (int i = 0; i < 16; ++i)
        {
            base[i] = (palette[i] >> 18) & 0x3f;
            base[i + 32] = (palette[i] >> 10) & 0x3f;
            base[i + 32 + 64] = (palette[i] >> 2) & 0x3f;
        }
        st->startRAMWR(0x2D);
        st->io.send(work->dataBuf, 128);
        st->endCS();
    }

    if (work->x == 0)
    {
        st->startRAMWR();
        work->x++;
    }

    if (st->double16 && work->srcLeft == 0 && work->x++ < (work->width << 1))
    {
        work->srcLeft = (work->height + 1) >> 1;
        if ((work->x & 1) == 0)
        {
            work->srcPtr -= work->srcLeft;
        }
    }

    // with the current image format in PXT the sendBytes cases never happen
    unsigned align = (unsigned)work->srcPtr & 3;
    if (work->srcLeft && align)
    {
        st->sendBytes(4 - align);
    }
    else if (work->srcLeft < 4)
    {
        if (work->srcLeft == 0)
        {
            st->endCS();
            Event(DEVICE_ID_DISPLAY, 100);
        }
        else
        {
            st->sendBytes(work->srcLeft);
        }
    }
    else
    {
        if (st->double16)
            st->sendWords(sizeof(work->dataBuf) / 8);
        else
            st->sendWords((sizeof(work->dataBuf) / (3 * 4)) * 4);
    }
}

void ST7735::startTransfer(unsigned size)
{
    io.startSend(work->dataBuf, size, (PVoidCallback)&ST7735::sendColorsStep, this);
}

void ST7735::startRAMWR(int cmd)
{
    if (cmd == 0)
        cmd = ST7735_RAMWR;
    cmdBuf[0] = cmd;
    sendCmd(cmdBuf, 1);

    setData();
    beginCS();
}

void ST7735::sendDone(Event)
{
    // this executes outside of interrupt context, so we don't get a race
    // with waitForSendDone
    work->inProgress = false;
    Event(DEVICE_ID_DISPLAY, 101);
}

void ST7735::waitForSendDone()
{
    if (work && work->inProgress)
        fiber_wait_for_event(DEVICE_ID_DISPLAY, 101);
}

int ST7735::setSleep(bool sleepMode)
{
    if (sleepMode == this->inSleepMode)
        return DEVICE_OK;

    if (sleepMode)
    {
        uint8_t cmd = ST7735_SLPIN;
        this->inSleepMode = true;
        waitForSendDone();
        sendCmd(&cmd, 1);
    }
    else
    {
        uint8_t cmd = ST7735_SLPOUT;
        sendCmd(&cmd, 1);
        fiber_sleep(120);
        this->inSleepMode = false;
    }

    return DEVICE_OK;
}

#define ENC16(r, g, b) (((r << 3) | (g >> 3)) & 0xff) | (((b | (g << 5)) & 0xff) << 8)

int ST7735::sendIndexedImage(const uint8_t *src, unsigned width, unsigned height, uint32_t *palette)
{
    if (!work)
    {
        work = new ST7735WorkBuffer;
        memset(work, 0, sizeof(*work));
        if (double16)
            for (int i = 0; i < 16; ++i)
            {
                uint16_t e = ENC16(i, i, i);
                work->expPalette[i] = e | (e << 16);
            }
        else
            for (int i = 0; i < 256; ++i)
                work->expPalette[i] = 0x1011 * (i & 0xf) | (0x110100 * (i >> 4));
        EventModel::defaultEventBus->listen(DEVICE_ID_DISPLAY, 100, this, &ST7735::sendDone);
    }

    if (work->inProgress || inSleepMode)
        return DEVICE_BUSY;

    work->paletteTable = palette;

    work->inProgress = true;
    work->srcPtr = src;
    work->width = width;
    work->height = height;
    work->srcLeft = (height + 1) >> 1;
    // when not scaling up, we don't care about where lines end
    if (!double16)
        work->srcLeft *= width;
    work->x = 0;

    sendColorsStep(this);

    return DEVICE_OK;
}

// we don't modify *buf, but it cannot be in flash, so no const as a hint
void ST7735::sendCmd(uint8_t *buf, int len)
{
    // make sure cmd isn't on stack
    if (buf != cmdBuf)
        memcpy(cmdBuf, buf, len);
    buf = cmdBuf;
    setCommand();
    beginCS();
    io.send(buf, 1);
    setData();
    len--;
    buf++;
    if (len > 0)
        io.send(buf, len);
    endCS();
}

void ST7735::sendCmdSeq(const uint8_t *buf)
{
    while (*buf)
    {
        cmdBuf[0] = *buf++;
        int v = *buf++;
        int len = v & ~DELAY;
        // note that we have to copy to RAM
        memcpy(cmdBuf + 1, buf, len);
        sendCmd(cmdBuf, len + 1);
        buf += len;
        if (v & DELAY)
        {
            fiber_sleep(*buf++);
        }
    }
}

void ST7735::setAddrWindow(int x, int y, int w, int h)
{
    int x2 = x + w - 1;
    int y2 = y + h - 1;
    uint8_t cmd0[] = {ST7735_RASET, (uint8_t)(x >> 8), (uint8_t)x, (uint8_t)(x2 >> 8), (uint8_t)x2};
    uint8_t cmd1[] = {ST7735_CASET, (uint8_t)(y >> 8), (uint8_t)y, (uint8_t)(y2 >> 8), (uint8_t)y2};
    sendCmd(cmd1, sizeof(cmd1));
    sendCmd(cmd0, sizeof(cmd0));
}

int ST7735::init()
{
    endCS();
    setData();

    fiber_sleep(10); // TODO check if delay needed
    sendCmdSeq(initCmds);

    return DEVICE_OK;
}

void ST7735::configure(uint8_t madctl, uint32_t frmctr1)
{
    uint8_t cmd0[] = {ST7735_MADCTL, madctl};
    uint8_t cmd1[] = {ST7735_FRMCTR1, (uint8_t)(frmctr1 >> 16), (uint8_t)(frmctr1 >> 8),
                      (uint8_t)frmctr1};
    if (madctl != 0xff)
        sendCmd(cmd0, sizeof(cmd0));
    if (frmctr1 != 0xffffff)
        sendCmd(cmd1, cmd1[3] == 0xff ? 3 : 4);
}

} // namespace codal