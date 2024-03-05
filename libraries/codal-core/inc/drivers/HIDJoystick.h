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

#ifndef DEVICE_HID_JOYSTICK_H
#define DEVICE_HID_JOYSTICK_H

#include "HID.h"

#if CONFIG_ENABLED(DEVICE_USB)

namespace codal
{
    typedef struct {
        int8_t throttle0;
        int8_t throttle1;

        int8_t x0;
        int8_t y0;

        int8_t x1;
        int8_t y1;

        uint16_t buttons;
    } __attribute__((packed)) HIDJoystickState;

    class USBHIDJoystick : public USBHID
    {

public:
        USBHIDJoystick();

        virtual int stdRequest(UsbEndpointIn &ctrl, USBSetup& setup);
        virtual const InterfaceInfo *getInterfaceInfo();

        int buttonDown(uint8_t b);
        int buttonUp(uint8_t b);

        int move(int8_t num, int8_t x, int8_t y);

        int setThrottle(uint8_t num, uint8_t val);

private:
        int sendReport();
    };
}


#endif

#endif