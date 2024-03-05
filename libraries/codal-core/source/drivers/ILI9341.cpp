#include "ILI9341.h"
#include "CodalFiber.h"
#include "CodalDmesg.h"

#define ILI9341_NOP 0x00     ///< No-op register
#define ILI9341_SWRESET 0x01 ///< Software reset register
#define ILI9341_RDDID 0x04   ///< Read display identification information
#define ILI9341_RDDST 0x09   ///< Read Display Status

#define ILI9341_SLPIN 0x10  ///< Enter Sleep Mode
#define ILI9341_SLPOUT 0x11 ///< Sleep Out
#define ILI9341_PTLON 0x12  ///< Partial Mode ON
#define ILI9341_NORON 0x13  ///< Normal Display Mode ON

#define ILI9341_RDMODE 0x0A     ///< Read Display Power Mode
#define ILI9341_RDMADCTL 0x0B   ///< Read Display MADCTL
#define ILI9341_RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT 0x0D   ///< Read Display Image Format
#define ILI9341_RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF 0x20   ///< Display Inversion OFF
#define ILI9341_INVON 0x21    ///< Display Inversion ON
#define ILI9341_GAMMASET 0x26 ///< Gamma Set
#define ILI9341_DISPOFF 0x28  ///< Display OFF
#define ILI9341_DISPON 0x29   ///< Display ON

#define ILI9341_CASET 0x2A ///< Column Address Set
#define ILI9341_PASET 0x2B ///< Page Address Set
#define ILI9341_RAMWR 0x2C ///< Memory Write
#define ILI9341_RAMRD 0x2E ///< Memory Read

#define ILI9341_PTLAR 0x30    ///< Partial Area
#define ILI9341_MADCTL 0x36   ///< Memory Access Control
#define ILI9341_VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT 0x3A   ///< COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1 0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2 0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3 0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR 0xB4  ///< Display Inversion Control
#define ILI9341_DFUNCTR 0xB6 ///< Display Function Control

#define ILI9341_PWCTR1 0xC0 ///< Power Control 1
#define ILI9341_PWCTR2 0xC1 ///< Power Control 2
#define ILI9341_PWCTR3 0xC2 ///< Power Control 3
#define ILI9341_PWCTR4 0xC3 ///< Power Control 4
#define ILI9341_PWCTR5 0xC4 ///< Power Control 5
#define ILI9341_VMCTR1 0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2 0xC7 ///< VCOM Control 2

#define ILI9341_RDID1 0xDA ///< Read ID 1
#define ILI9341_RDID2 0xDB ///< Read ID 2
#define ILI9341_RDID3 0xDC ///< Read ID 3
#define ILI9341_RDID4 0xDD ///< Read ID 4

#define ILI9341_GMCTRP1 0xE0 ///< Positive Gamma Correction
#define ILI9341_GMCTRN1 0xE1 ///< Negative Gamma Correction
//#define ILI9341_PWCTR6     0xFC


// clang-format off
static const uint8_t initcmd[] = {

#if 0
    // Parameters based on https://github.com/juj/fbcp-ili9341
    0x01/*Software Reset*/, 0x80, 5,
    0x28/*Display OFF*/, 0,
    // The following appear in ILI9341 Data Sheet v1.11 (2011/06/10), but not in older v1.02 (2010/12/06).
    0xCB/*Power Control A*/, 5, 0x39/*Reserved*/, 0x2C/*Reserved*/, 0x00/*Reserved*/, 0x34/*REG_VD=1.6V*/, 0x02/*VBC=5.6V*/,
    0xCF/*Power Control B*/, 3, 0x00/*Always Zero*/, 0xC1/*Power Control=0,DRV_ena=0,PCEQ=1*/, 0x30,/*DC_ena=1*/ // Not sure what the effect is, set to default as per ILI9341 Application Notes v0.6 (2011/03/11) document (which is not apparently same as default at power on).
    0xE8/*Driver Timing Control A*/, 3, 0x85, 0x00, 0x78, // Not sure what the effect is, set to default as per ILI9341 Application Notes v0.6 (2011/03/11) document (which is not apparently same as default at power on).
    0xEA/*Driver Timing Control B*/, 2, 0x00, 0x00, // Not sure what the effect is, set to default as per ILI9341 Application Notes v0.6 (2011/03/11) document (which is not apparently same as default at power on).
    0xED/*Power On Sequence Control*/, 4, 0x64, 0x03, 0x12, 0x81, // Not sure what the effect is, set to default as per ILI9341 Application Notes v0.6 (2011/03/11) document (which is not apparently same as default at power on).
    // The following appear also in old ILI9341 Data Sheet v1.02 (2010/12/06).
    0xC0/*Power Control 1*/, 1, 0x23,/*VRH=4.60V*/ // Set the GVDD level, which is a reference level for the VCOM level and the grayscale voltage level.
    0xC1/*Power Control 2*/, 1, 0x10,/*AVCC=VCIx2,VGH=VCIx7,VGL=-VCIx4*/ // Sets the factor used in the step-up circuits. To reduce power consumption, set a smaller factor.
    0xC5/*VCOM Control 1*/, 2, 0x3e/*VCOMH=4.250V*/, 0x28,/*VCOML=-1.500V*/ // Adjusting VCOM 1 and 2 can control display brightness
    0xC7/*VCOM Control 2*/, 1, 0x86,/*VCOMH=VMH-58,VCOML=VML-58*/

    0x36/*MADCTL: Memory Access Control*/  , 1, 0x48,             // Memory Access Control

    0x20/*Display Inversion OFF*/, 0,
    0x3A/*COLMOD: Pixel Format Set*/, 1, 0x55,/*DPI=16bits/pixel,DBI=16bits/pixel*/

    // According to spec sheet, display frame rate in 4-wire SPI "internal clock mode" is computed with the following formula:
    // frameRate = 615000 / [ (pow(2,DIVA) * (320 + VFP + VBP) * RTNA ]
    // where
    // - DIVA is clock division ratio, 0 <= DIVA <= 3; so pow(2,DIVA) is either 1, 2, 4 or 8.
    // - RTNA specifies the number of clocks assigned to each horizontal scanline, and must follow 16 <= RTNA <= 31.
    // - VFP is vertical front porch, number of idle sleep scanlines before refreshing a new frame, 2 <= VFP <= 127.
    // - VBP is vertical back porch, number of idle sleep scanlines after refreshing a new frame, 2 <= VBP <= 127.

    // Max refresh rate then is with DIVA=0, VFP=2, VBP=2 and RTNA=16:
    // maxFrameRate = 615000 / (1 * (320 + 2 + 2) * 16) = 118.63 Hz

    // To get 60fps, set DIVA=0, RTNA=31, VFP=2 and VBP=2:
    // minFrameRate = 615000 / (8 * (320 + 2 + 2) * 31) = 61.23 Hz

    // It seems that in internal clock mode, horizontal front and back porch settings (HFP, BFP) are ignored(?)

    0xB1/*Frame Rate Control (In Normal Mode/Full Colors)*/, 2, 0x00/*DIVA=fosc*/, 0x18,/*RTNA(Frame Rate)*/

    0xB6/*Display Function Control*/, 1, 0x08,/*PTG=Interval Scan,PT=V63/V0/VCOML/VCOMH*/ 0x82/*REV=1(Normally white),ISC(Scan Cycle)=5 frames*/, 1, 0x27,/*LCD Driver Lines=320*/
    0xF2/*Enable 3G*/, 1, 0x02,/*False*/ // This one is present only in ILI9341 Data Sheet v1.11 (2011/06/10)
    0x26/*Gamma Set*/, 1, 0x01/*Gamma curve 1 (G2.2)*/,
    0xE0/*Positive Gamma Correction*/, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    0xE1/*Negative Gamma Correction*/, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    0x11, 0x80, /*Sleep Out*/
    120,
    /*Display ON*/0x29, 0x80,
    120,
    0, 0,
#else

  // Parameters based on https://github.com/adafruit/Adafruit_ILI9341
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI9341_PWCTR1  , 1, 0x23,             // Power control VRH[5:0]
  ILI9341_PWCTR2  , 1, 0x10,             // Power control SAP[2:0];BT[3:0]
  ILI9341_VMCTR1  , 2, 0x3e, 0x28,       // VCM control
  ILI9341_VMCTR2  , 1, 0x86,             // VCM control2
  ILI9341_MADCTL  , 1, 0x08,             // Memory Access Control
  ILI9341_VSCRSADD, 1, 0x00,             // Vertical scroll zero
  ILI9341_PIXFMT  , 1, 0x55,
  ILI9341_FRMCTR1 , 2, 0x00, 0x18,
  ILI9341_DFUNCTR , 3, 0x08, 0x82, 0x27, // Display Function Control
  0xF2, 1, 0x00,                         // 3Gamma Function Disable
  ILI9341_GAMMASET , 1, 0x01,             // Gamma curve selected
  ILI9341_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI9341_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT  , 0x80,                // Exit Sleep
    120,
  ILI9341_DISPON  , 0x80,                // Display on
    120,
  0x00, 0x00,                                // End of list
#endif

};
// clang-format on

namespace codal
{

ILI9341::ILI9341(ScreenIO &io, Pin &cs, Pin &dc) : ST7735(io, cs, dc)
{
    double16 = true;
}

int ILI9341::init()
{
    endCS();
    setData();

    fiber_sleep(10);
    sendCmdSeq(initcmd);

    return DEVICE_OK;
}

} // namespace codal