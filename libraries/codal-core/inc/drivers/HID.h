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

#ifndef DEVICE_HID_H
#define DEVICE_HID_H

#include "CodalUSB.h"

#if CONFIG_ENABLED(DEVICE_USB)

#define HID_REQUEST_GET_REPORT 0x01
#define HID_REQUEST_GET_IDLE 0x02
#define HID_REQUEST_GET_PROTOCOL 0x03
#define HID_REQUEST_SET_REPORT 0x09
#define HID_REQUEST_SET_IDLE 0x0A
#define HID_REQUEST_SET_PROTOCOL 0x0B

namespace codal
{
    typedef struct {
        uint8_t len;
        uint8_t type; // 0x21
        uint16_t hidBCD; // 0x100
        uint8_t countryCode;
        uint8_t numDesc;
        uint8_t reportDescType; // 0x22
        uint16_t sizeOfReport;
    } __attribute__((packed)) HIDReportDescriptor;

    class USBHID : public CodalUSBInterface
    {
        public:
        USBHID();

        virtual int classRequest(UsbEndpointIn &ctrl, USBSetup& setup);
        virtual int stdRequest(UsbEndpointIn &ctrl, USBSetup& setup);
        virtual int endpointRequest();
        virtual const InterfaceInfo *getInterfaceInfo();
    };
}


#endif

#endif
