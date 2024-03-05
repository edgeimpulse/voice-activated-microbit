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

#include "HIDJoystick.h"
#include "HID.h"

#if CONFIG_ENABLED(DEVICE_USB)

using namespace codal;

//this descriptor must be stored in RAM
static char HIDJoystickDescriptor[] = {
	 0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	 0x09, 0x05, // USAGE (Game Pad)
	 0xa1, 0x01, // COLLECTION (Application)
	 0x05, 0x02, // USAGE_PAGE (Simulation Controls)
	 0x09, 0xbb, // USAGE (Throttle)
	 0x15, 0x00, // LOGICAL_MINIMUM (0)
	 0x25, 0x1f, // LOGICAL_MAXIMUM (31)
	 0x75, 0x08, // REPORT_SIZE (8)
	 0x95, 0x01, // REPORT_COUNT (1)
	 0x81, 0x02, // INPUT (Data,Var,Abs)
	 0x05, 0x02, // USAGE_PAGE (Simulation Controls)
	 0x09, 0xb0, // USAGE (Rudder)
	 0x15, 0x00, // LOGICAL_MINIMUM (0)
	 0x25, 0x1f, // LOGICAL_MAXIMUM (31)
	 0x75, 0x08, // REPORT_SIZE (8)
	 0x95, 0x01, // REPORT_COUNT (1)
	 0x81, 0x02, // INPUT (Data,Var,Abs)
	 0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	 0xa1, 0x00, // COLLECTION (Physical)
	 0x09, 0x30, // USAGE (X)
	 0x09, 0x31, // USAGE (Y)
	 0x09, 0x32, // USAGE (Z)
	 0x09, 0x35, // USAGE (Rz)
	 0x15, 0x81, // LOGICAL_MINIMUM (-127)
	 0x25, 0x7f, // LOGICAL_MAXIMUM (127)
	 0x75, 0x08, // REPORT_SIZE (8)
	 0x95, 0x04, // REPORT_COUNT (4)
	 0x81, 0x02, // INPUT (Data,Var,Abs)
	 0x05, 0x09, // USAGE_PAGE (Button)
	 0x19, 0x01, // USAGE_MINIMUM (Button 1)
	 0x29, 0x10, // USAGE_MAXIMUM (Button 16)
	 0x15, 0x00, // LOGICAL_MINIMUM (0)
	 0x25, 0x01, // LOGICAL_MAXIMUM (1)
	 0x75, 0x01, // REPORT_SIZE (1)
	 0x95, 0x10, // REPORT_COUNT (16)
	 0x81, 0x02, // INPUT (Data,Var,Abs)
	 0xc0, // END_COLLECTION
	 0xc0 // END_COLLECTION
};

static const HIDReportDescriptor reportDesc = {
	9,
	0x21,                  // HID
	0x101,                 // hidbcd 1.01
	0x00,                  // country code
	0x01,                  // num desc
	0x22,                  // report desc type
	sizeof(HIDJoystickDescriptor),
};

static const InterfaceInfo ifaceInfo = {
	&reportDesc,
	sizeof(reportDesc),
	1,
	{
		1,    // numEndpoints
		0x03, /// class code - HID
		0x01, // subclass (boot interface)
		0x02, // protocol (joystick)
		0x00, //
		0x00, //
	},
	{USB_EP_TYPE_INTERRUPT, 1},
	{USB_EP_TYPE_INTERRUPT, 1},
};

static HIDJoystickState joystickState = {
	0, 0, 0, 0, 0, 0, 0,
};

USBHIDJoystick::USBHIDJoystick() : USBHID()
{

}

int USBHIDJoystick::stdRequest(UsbEndpointIn &ctrl, USBSetup &setup)
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
			return ctrl.write(HIDJoystickDescriptor, sizeof(HIDJoystickDescriptor));
		}
	}
	return DEVICE_NOT_SUPPORTED;
}

const InterfaceInfo *USBHIDJoystick::getInterfaceInfo()
{
	return &ifaceInfo;
}

int USBHIDJoystick::buttonDown(uint8_t b)
{
	uint16_t btn = (1UL << b);
	if( joystickState.buttons & btn )
		return DEVICE_OK;
	else{
		joystickState.buttons |= btn;
		return sendReport();
	}
}

int USBHIDJoystick::buttonUp(uint8_t b)
{
	uint16_t btn = (1UL << b);
	if( !(joystickState.buttons & btn) )
		return DEVICE_OK;
	else{
		joystickState.buttons &= ~(btn);
		return sendReport();
	}
}

int USBHIDJoystick::move(int8_t num, int8_t x, int8_t y)
{
	switch(num){
		case 0:
			joystickState.x0 = x;
			joystickState.y0 = y;
			break;
		case 1:
			joystickState.x1 = x;
			joystickState.y1 = y;
			break;
		default:
			return DEVICE_INVALID_PARAMETER;
			break;
	}
	return sendReport();
}

int USBHIDJoystick::setThrottle(uint8_t num, uint8_t val)
{
	if(val > 31)
		return DEVICE_INVALID_PARAMETER;
	switch(num){
		case 0:
			joystickState.throttle0 = val;
			break;
		case 1:
			joystickState.throttle1 = val;
			break;
		default:
			return DEVICE_INVALID_PARAMETER;
			break;
	}
	return sendReport();
}

int USBHIDJoystick::sendReport()
{
	uint8_t report[sizeof(HIDJoystickState)];
	memcpy(report, &joystickState, sizeof(HIDJoystickState));

	return in->write(report, sizeof(report));
}

#endif
