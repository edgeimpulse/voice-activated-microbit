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

#include "HIDMouse.h"
#include "HID.h"

#if CONFIG_ENABLED(DEVICE_USB)

using namespace codal;

//this descriptor must be stored in RAM
static char hidMouseDescriptor[] = {
	0x05, 0x01, 		//Usage Page: Generic Desktop Controls
	0x09, 0x02, 		//Usage: Mouse (2)
	0xA1, 0x01, 		//Collection: Application
	0x09, 0x01, 		//Usage: Pointer (1)
	0xA1, 0x00, 		//Collection: Physical
	0x05, 0x09, 		//Usage Page: Button (9)
	0x19, 0x01, 		//Usage Minimum: Button 1
	0x29, 0x03, 		//Usage Maximum: Button 3
	0x15, 0x00, 		//Logical Minimum: 0
	0x25, 0x01, 		//Logical Maximum: 1
	0x95, 0x03, 		//Report Count: 3
	0x75, 0x01, 		//Report Size: 1
	0x81, 0x02, 		//Input: Data (2)
	0x95, 0x01, 		//Report Count: 1
	0x75, 0x05, 		//Report Size: 5
	0x81, 0x01, 		//Input: Constant (1)
	0x05, 0x01, 		//Usage Page: Generic Desktop Controls
	0x09, 0x30, 		//Usage: X
	0x09, 0x31, 		//Usage: Y
	0x09, 0x38, 		//Usage: Wheel
	0x15, 0x81, 		//Logical Minimum: -127
	0x25, 0x7f, 		//Logical Maximum: 127
	0x75, 0x08, 		//Report Size: 8
	0x95, 0x03, 		//Report Count: 3
	0x81, 0x06, 		//Input: Data (6)
	0xC0, 				//End collection
	0xC0 				//End collection
};

static const HIDReportDescriptor reportDesc = {
	9,
	0x21,                  // HID
	0x101,                 // hidbcd 1.01
	0x00,                  // country code
	0x01,                  // num desc
	0x22,                  // report desc type
	sizeof(hidMouseDescriptor),
};

static const InterfaceInfo ifaceInfo = {
	&reportDesc,
	sizeof(reportDesc),
	1,
	{
		1,    // numEndpoints
		0x03, /// class code - HID
		0x01, // subclass (boot interface)
		0x02, // protocol (Mouse)
		0x00, //
		0x00, //
	},
	{USB_EP_TYPE_INTERRUPT, 1},
	{USB_EP_TYPE_INTERRUPT, 1},
};

static HIDMouseState mouseState = {
	0, 0, 0, 0, 0, 0, 0
};

USBHIDMouse::USBHIDMouse() : USBHID()
{

}

int USBHIDMouse::stdRequest(UsbEndpointIn &ctrl, USBSetup &setup)
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
			return ctrl.write(hidMouseDescriptor, sizeof(hidMouseDescriptor));
		}
	}
	return DEVICE_NOT_SUPPORTED;
}

const InterfaceInfo *USBHIDMouse::getInterfaceInfo()
{
	return &ifaceInfo;
}

int USBHIDMouse::buttonDown(USBHIDMouseButton b)
{
	if(mouseState.buttons.reg & b)
		return DEVICE_OK;
	else{
		mouseState.buttons.reg |= b;
		return sendReport();
	}
}

int USBHIDMouse::buttonUp(USBHIDMouseButton b)
{
	if( !(mouseState.buttons.reg & b) )
		return DEVICE_OK;
	else{
		mouseState.buttons.reg &= ~(b);
		return sendReport();
	}
}

int USBHIDMouse::move(int8_t x, int8_t y)
{
	mouseState.xMovement = x;
	mouseState.yMovement = y;
	return sendReport();
}

int USBHIDMouse::moveWheel(int8_t w)
{
	mouseState.wheelMovement = w;
	return sendReport();
}

int USBHIDMouse::sendReport()
{
	uint8_t report[sizeof(HIDMouseState)];
	memcpy(report, &mouseState, sizeof(HIDMouseState));

	//movements are relative
	mouseState.xMovement = 0;
	mouseState.yMovement = 0;
	mouseState.wheelMovement = 0;

	return in->write(report, sizeof(report));
}

#endif
