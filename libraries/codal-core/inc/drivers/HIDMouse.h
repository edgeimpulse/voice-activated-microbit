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

#ifndef DEVICE_HID_MOUSE_H
#define DEVICE_HID_MOUSE_H

#include "HID.h"

#if CONFIG_ENABLED(DEVICE_USB)

namespace codal
{
    typedef enum {
        HID_MOUSE_LEFT = 0x01,
        HID_MOUSE_RIGHT = 0x02,
        HID_MOUSE_MIDDLE = 0x04,
    } USBHIDMouseButton;

    typedef union {
        struct {
            bool rightButton:1;
            bool middleButton:1;
            bool leftButton:1;
            uint8_t reserved:5;
        } bit;
        uint8_t reg;
    } HIDMouseButtons;

    typedef struct {
        HIDMouseButtons buttons;

        int8_t xMovement;
        int8_t yMovement;
        int8_t wheelMovement;

    } __attribute__((packed)) HIDMouseState;

    class USBHIDMouse : public USBHID
    {

public:
        USBHIDMouse();

        virtual int stdRequest(UsbEndpointIn &ctrl, USBSetup& setup);
        virtual const InterfaceInfo *getInterfaceInfo();

        int buttonDown(USBHIDMouseButton b);
        int buttonUp(USBHIDMouseButton b);

        int move(int8_t x, int8_t y);
        int moveWheel(int8_t w);

private:
        int sendReport();
    };
}


#endif

#endif