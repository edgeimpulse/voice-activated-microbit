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

#include "HID.h"

#if CONFIG_ENABLED(DEVICE_USB)

using namespace codal;

static const char hidDescriptor[] = {
    0x06, 0x00, 0xFF, // usage page vendor #0
    0x09, 0x01,       // usage 1
    0xA1, 0x01,       // collection - application
    0x15, 0x00,       // logical min 0
    0x26, 0xFF, 0x00, // logical max 255
    0x75, 8,          // report size 8
    0x95, 64,         // report count 64
    0x09, 0x01,       // usage 1
    0x81, 0x02,       // input: data, variable, absolute
    0x95, 64,         // report count 64
    0x09, 0x01,       // usage 1
    0x91, 0x02,       // output: data, variable, absolute
    0x95, 1,          // report count 1
    0x09, 0x01,       // usage 1
    0xB1, 0x02,       // feature: data, variable, absolute
    0xC0,             // end
};

static const HIDReportDescriptor reportDesc = {
    9,
    0x21,                  // HID
    0x100,                 // hidbcd 1.00
    0x00,                  // country code
    0x01,                  // num desc
    0x22,                  // report desc type
    sizeof(hidDescriptor), // size of 0x22
};

static const InterfaceInfo ifaceInfo = {
    &reportDesc,
    sizeof(reportDesc),
    1,
    {
        2,    // numEndpoints
        0x03, /// class code - HID
        0x00, // subclass
        0x00, // protocol
        0x00, //
        0x00, //
    },
    {USB_EP_TYPE_INTERRUPT, 1},
    {USB_EP_TYPE_INTERRUPT, 1},
};

USBHID::USBHID() : CodalUSBInterface()
{
}

int USBHID::stdRequest(UsbEndpointIn &ctrl, USBSetup &setup)
{
    if (setup.bRequest == USB_REQ_GET_DESCRIPTOR)
    {
        if (setup.wValueH == 0x21)
        {
            InterfaceDescriptor tmp;
            fillInterfaceInfo(&tmp);
            return ctrl.write(&tmp, sizeof(tmp));
        }
        else if (setup.wValueH == 0x22)
        {
            return ctrl.write(hidDescriptor, sizeof(hidDescriptor));
        }
    }
    return DEVICE_NOT_SUPPORTED;
}

int USBHID::endpointRequest()
{
    uint8_t buf[64];
    int len = out->read(buf, sizeof(buf));
    if (len <= 0)
        return len;

    for (int i = 1; i < 4; ++i)
    {
        buf[i] ^= 'a' - 'A';
    }

    // this should echo-back serial
    return in->write(buf, sizeof(buf));
}

const InterfaceInfo *USBHID::getInterfaceInfo()
{
    return &ifaceInfo;
}

int USBHID::classRequest(UsbEndpointIn &ctrl, USBSetup &setup)
{
    uint8_t buf[8] = {0};

    switch (setup.bRequest)
    {
    case HID_REQUEST_GET_PROTOCOL:
    case HID_REQUEST_GET_IDLE:
    case HID_REQUEST_GET_REPORT:
        return ctrl.write(buf, sizeof(buf));

    case HID_REQUEST_SET_IDLE:
    case HID_REQUEST_SET_REPORT:
    case HID_REQUEST_SET_PROTOCOL:
        return ctrl.write(buf, 0);
    }

    return DEVICE_NOT_SUPPORTED;
}

#endif
