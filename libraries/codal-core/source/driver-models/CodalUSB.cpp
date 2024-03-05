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

#include "CodalUSB.h"

#if CONFIG_ENABLED(DEVICE_USB)

// If DEVICE_USB_ENDPOINT_SHARING in and out endpoints will always get the same index.
// This works on STM32F4. Can define on SAMD and elsewhere when tested.
#if defined(DEVICE_USB_ENDPOINT_SHARING)
#define NUM_ENDPOINTS(x) ((x) > 1 ? 1 : (x))
#else
#define NUM_ENDPOINTS(x) (x)
#endif

#include "ErrorNo.h"
#include "CodalDmesg.h"
#include "codal_target_hal.h"

#define send(p, l) ctrlIn->write(p, l)

CodalUSB *CodalUSB::usbInstance = NULL;

// #define LOG DMESG
#define LOG(...)

static uint8_t usb_initialised = 0;
// usb_20.pdf
static uint8_t usb_status = 0;
// static uint8_t usb_suspended = 0; // copy of UDINT to check SUSPI and WAKEUPI bits
static uint8_t usb_configured = 0;

static const ConfigDescriptor static_config = {9, 2, 0, 0, 1, 0, USB_CONFIG_BUS_POWERED, 0};

static const DeviceDescriptor default_device_desc = {
    0x12, // bLength
    0x01, // bDescriptorType
#if CONFIG_ENABLED(DEVICE_WEBUSB)
    0x0210, // bcdUSBL
#else
    0x0200, // bcdUSBL
#endif

#if 0
    // This should be only used if we implement USB Serial (CDC) with IAD descriptors
    0xEF,            // bDeviceClass:    Misc
    0x02,            // bDeviceSubclass:
    0x01,            // bDeviceProtocol:
#else
    // Class etc specified per-interface
    0x00, 0x00, 0x00,
#endif
    0x40,            // bMaxPacketSize0
    USB_DEFAULT_VID, //
    USB_DEFAULT_PID, //
    0x4202,          // bcdDevice - leave unchanged for the HF2 to work
    0x01,            // iManufacturer
    0x02,            // iProduct
    0x03,            // SerialNumber
    0x01             // bNumConfigs
};

static const char *default_strings[] = {
    "CoDAL Devices",
    "Generic CoDAL device",
    "4242",
};

#if CONFIG_ENABLED(DEVICE_WEBUSB)
#define VENDOR_WEBUSB 0x40
#define VENDOR_MS20 0x41

#define WINUSB_SIZE()                                                                              \
    (sizeof(msOS20Descriptor) + numWebUSBInterfaces * sizeof(msOS20FunctionDescriptor))

static const uint8_t bosDescriptor[] = {
    0x05,       // Length
    0x0F,       // Binary Object Store descriptor
    0x39, 0x00, // Total length
    0x02,       // Number of device capabilities

    // WebUSB Platform Capability descriptor (bVendorCode == 0x01).
    0x18,                                           // Length
    0x10,                                           // Device Capability descriptor
    0x05,                                           // Platform Capability descriptor
    0x00,                                           // Reserved
    0x38, 0xB6, 0x08, 0x34, 0xA9, 0x09, 0xA0, 0x47, // WebUSB GUID
    0x8B, 0xFD, 0xA0, 0x76, 0x88, 0x15, 0xB6, 0x65, // WebUSB GUID
    0x00, 0x01,                                     // Version 1.0
    VENDOR_WEBUSB,                                  // Vendor request code
    0x00,                                           // landing page

    0x1C,                                           // Length
    0x10,                                           // Device Capability descriptor
    0x05,                                           // Platform Capability descriptor
    0x00,                                           // Reserved
    0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C, // MS OS 2.0 GUID
    0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F, // MS OS 2.0 GUID
    0x00, 0x00, 0x03, 0x06,                         // Windows version
    0xff, 0xff,  // Descriptor set length; bosDescriptor[sizeof(bosDescriptor)-4]
    VENDOR_MS20, // Vendor request code
    0x00         // Alternate enumeration code
};

static const uint8_t msOS20FunctionDescriptor[] = {
    // Microsoft OS 2.0 function subset header
    0x08, 0x00, // Descriptor size (8 bytes)
    0x02, 0x00, // MS OS 2.0 function subset header
    0xff,       // first interface no; msOS20FunctionDescriptor[4]
    0x00,       // Reserved
    160, 0x00,  // Size, MS OS 2.0 function subset

    // Microsoft OS 2.0 compatible ID descriptor (table 13)
    20, 0x00,                     // wLength
    0x03, 0x00,                   // MS_OS_20_FEATURE_COMPATIBLE_ID
    'W', 'I', 'N', 'U', 'S', 'B', //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // interface guids
    132, 0, 4, 0, 7, 0,
    //
    42, 0,
    //
    'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0,
    'a', 0, 'c', 0, 'e', 0, 'G', 0, 'U', 0, 'I', 0, 'D', 0, 's', 0, 0, 0,
    //
    80, 0,
    //
    '{', 0, '9', 0, '2', 0, 'C', 0, 'E', 0, '6', 0, '4', 0, '6', 0, '2', 0, '-', 0, '9', 0, 'C', 0,
    '7', 0, '7', 0, '-', 0, '4', 0, '6', 0, 'F', 0, 'E', 0, '-', 0, '9', 0, '3', 0, '3', 0, 'B', 0,
    '-', 0, '3', 0, '1', 0, 'C', 0, 'B', 0, '9', 0, 'C', 0, '5', 0, 'A', 0, 'A', 0, '3', 0, 'B', 0,
    'A', 0, '}', 0, 0, 0, 0, 0};

static const uint8_t msOS20Descriptor[] = {
    // Microsoft OS 2.0 descriptor set header (table 10)
    0x0A, 0x00,             // Descriptor size (10 bytes)
    0x00, 0x00,             // MS OS 2.0 descriptor set header
    0x00, 0x00, 0x03, 0x06, // Windows version (8.1) (0x06030000)
    0xff, 0xff,             // Size, MS OS 2.0 descriptor set
};
#endif

static const InterfaceInfo codalDummyIfaceInfo = {
    NULL,
    0,
    0,
    {
        0,    // numEndpoints
        0xff, /// class code - vendor-specific
        0xff, // subclass
        0xff, // protocol
        0x00, // string
        0x00, // alt
    },
    {0, 0},
    {0, 0},
};

const InterfaceInfo *CodalDummyUSBInterface::getInterfaceInfo()
{
    return &codalDummyIfaceInfo;
}

CodalUSB::CodalUSB()
{
    usbInstance = this;
    endpointsUsed = 1; // CTRL endpoint
    ctrlIn = NULL;
    ctrlOut = NULL;
    numStringDescriptors = sizeof(default_strings) / sizeof(default_strings[0]);
    stringDescriptors = default_strings;
    deviceDescriptor = &default_device_desc;
    startDelayCount = 1;
    interfaces = NULL;
    numWebUSBInterfaces = 0;
    maxPower = 50; // 100mA; if set to 500mA can't connect to iOS devices
}

void CodalUSBInterface::fillInterfaceInfo(InterfaceDescriptor *descp)
{
    const InterfaceInfo *info = this->getInterfaceInfo();
    InterfaceDescriptor desc = {
        sizeof(InterfaceDescriptor),
        4, // type
        this->interfaceIdx,
        info->iface.alternate,
        info->iface.numEndpoints,
        info->iface.interfaceClass,
        info->iface.interfaceSubClass,
        info->iface.protocol,
        info->iface.iInterfaceString,
    };
    *descp = desc;
}

int CodalUSB::sendConfig()
{
    const InterfaceInfo *info;
    int numInterfaces = 0;
    int clen = sizeof(ConfigDescriptor);

    // calculate the total size of our interfaces.
    for (CodalUSBInterface *iface = interfaces; iface; iface = iface->next)
    {
        info = iface->getInterfaceInfo();
        clen += sizeof(InterfaceDescriptor) +
                info->iface.numEndpoints * sizeof(EndpointDescriptor) +
                info->supplementalDescriptorSize;
        numInterfaces++;
    }

    uint8_t *buf = new uint8_t[clen];
    memcpy(buf, &static_config, sizeof(ConfigDescriptor));
    ((ConfigDescriptor *)buf)->clen = clen;
    ((ConfigDescriptor *)buf)->numInterfaces = numInterfaces;
    ((ConfigDescriptor *)buf)->maxPower = maxPower;
    clen = sizeof(ConfigDescriptor);

#define ADD_DESC(desc)                                                                             \
    memcpy(buf + clen, &desc, sizeof(desc));                                                       \
    clen += sizeof(desc)

    // send our descriptors
    for (CodalUSBInterface *iface = interfaces; iface; iface = iface->next)
    {
        info = iface->getInterfaceInfo();
        InterfaceDescriptor desc;
        iface->fillInterfaceInfo(&desc);
        ADD_DESC(desc);

        if (info->supplementalDescriptorSize)
        {
            memcpy(buf + clen, info->supplementalDescriptor, info->supplementalDescriptorSize);
            clen += info->supplementalDescriptorSize;
        }

        if (info->iface.numEndpoints == 0)
        {
            // OK
        }

        if (info->iface.numEndpoints >= 1)
        {
            EndpointDescriptor epdescIn = {
                sizeof(EndpointDescriptor),
                5, // type
                (uint8_t)(0x80 | iface->in->ep),
                info->epIn.attr,
                USB_MAX_PKT_SIZE,
                info->epIn.interval,
            };
            ADD_DESC(epdescIn);
        }

        if (info->iface.numEndpoints >= 2)
        {
            EndpointDescriptor epdescOut = {
                sizeof(EndpointDescriptor),
                5, // type
                iface->out->ep,
                info->epIn.attr,
                USB_MAX_PKT_SIZE,
                info->epIn.interval,
            };
            ADD_DESC(epdescOut);
        }

        if (info->iface.numEndpoints >= 3)
        {
            usb_assert(0);
        }
    }

    usb_assert(clen == ((ConfigDescriptor *)buf)->clen);

    send(buf, clen);

    delete buf;

    return DEVICE_OK;
}

// languageID - United States
static const uint8_t string0[] = {4, 3, 9, 4};

int CodalUSB::sendDescriptors(USBSetup &setup)
{
    uint8_t type = setup.wValueH;

    if (type == USB_CONFIGURATION_DESCRIPTOR_TYPE)
        return sendConfig();

    if (type == USB_DEVICE_DESCRIPTOR_TYPE)
        return send(deviceDescriptor, sizeof(DeviceDescriptor));

#if CONFIG_ENABLED(DEVICE_WEBUSB)
    if (type == USB_BOS_DESCRIPTOR_TYPE && numWebUSBInterfaces > 0)
    {
        uint8_t buf[sizeof(bosDescriptor)];
        memcpy(buf, bosDescriptor, sizeof(buf));
        buf[sizeof(bosDescriptor) - 4] = WINUSB_SIZE() & 0xff;
        buf[sizeof(bosDescriptor) - 3] = WINUSB_SIZE() >> 8;
        return send(buf, sizeof(buf));
    }
#endif

    if (type == USB_STRING_DESCRIPTOR_TYPE)
    {
        // check if we exceed our bounds.
        if (setup.wValueL > numStringDescriptors)
            return DEVICE_NOT_SUPPORTED;

        if (setup.wValueL == 0)
            return send(string0, sizeof(string0));

        StringDescriptor desc;

        const char *str = stringDescriptors[setup.wValueL - 1];
        if (!str)
            return DEVICE_NOT_SUPPORTED;

        desc.type = 3;
        uint32_t len = strlen(str) * 2 + 2;
        desc.len = len;

        usb_assert(len <= sizeof(desc));

        int i = 0;
        while (*str)
            desc.data[i++] = *str++;

        // send the string descriptor the host asked for.
        return send(&desc, desc.len);
    }
    else
    {
        return interfaceRequest(setup, false);
    }

    return DEVICE_NOT_SUPPORTED;
}

CodalUSB *CodalUSB::getInstance()
{
    if (usbInstance == NULL)
        usbInstance = new CodalUSB;

    return usbInstance;
}

int CodalUSB::add(CodalUSBInterface &interface)
{
    usb_assert(!usb_configured);

    uint8_t epsConsumed = NUM_ENDPOINTS(interface.getInterfaceInfo()->allocateEndpoints);

    if (endpointsUsed + epsConsumed > DEVICE_USB_ENDPOINTS)
        return DEVICE_NO_RESOURCES;

    interface.interfaceIdx = 0xff;

    CodalUSBInterface *iface;
    interface.next = NULL;

    for (iface = interfaces; iface; iface = iface->next)
    {
        if (!iface->next)
            break;
#if CONFIG_ENABLED(DEVICE_WEBUSB)
        // adding a non-web interface - it comes before all web interfaces
        if (!interface.enableWebUSB() && iface->next->enableWebUSB())
        {
            interface.next = iface->next;
            break;
        }
#endif
    }

    if (iface)
        iface->next = &interface;
    else
        interfaces = &interface;

    endpointsUsed += epsConsumed;

    return DEVICE_OK;
}

int CodalUSB::isInitialised()
{
    return usb_initialised > 0;
}

int CodalUSB::interfaceRequest(USBSetup &setup, bool isClass)
{
    int ifaceIdx = -1;
    int epIdx = -1;

    if ((setup.bmRequestType & USB_REQ_DESTINATION) == USB_REQ_INTERFACE)
        ifaceIdx = setup.wIndex & 0xff;
    else if ((setup.bmRequestType & USB_REQ_DESTINATION) == USB_REQ_ENDPOINT)
        epIdx = setup.wIndex & 0x7f;

    LOG("iface req: ifaceIdx=%d epIdx=%d", ifaceIdx, epIdx);

    for (CodalUSBInterface *iface = interfaces; iface; iface = iface->next)
    {
        if (iface->interfaceIdx == ifaceIdx ||
            ((iface->in && iface->in->ep == epIdx) || (iface->out && iface->out->ep == epIdx)))
        {
            int res =
                isClass ? iface->classRequest(*ctrlIn, setup) : iface->stdRequest(*ctrlIn, setup);
            LOG("iface req res=%d", res);
            if (res == DEVICE_OK)
                return DEVICE_OK;
        }
    }

    return DEVICE_NOT_SUPPORTED;
}

#define sendzlp() send(&usb_status, 0)
#define stall ctrlIn->stall

void CodalUSB::setupRequest(USBSetup &setup)
{
    LOG("SETUP Req=%x type=%x val=%x:%x idx=%x len=%d", setup.bRequest, setup.bmRequestType,
        setup.wValueH, setup.wValueL, setup.wIndex, setup.wLength);

    int status = DEVICE_OK;

    // Standard Requests
    uint16_t wValue = (setup.wValueH << 8) | setup.wValueL;
    uint8_t request_type = setup.bmRequestType;
    uint16_t wStatus = 0;

    ctrlIn->wLength = setup.wLength;

    if ((request_type & USB_REQ_TYPE) == USB_REQ_STANDARD)
    {
        LOG("STD REQ");
        switch (setup.bRequest)
        {
        case USB_REQ_GET_STATUS:
            LOG("STA");
            if (request_type == (USB_REQ_DEVICETOHOST | USB_REQ_STANDARD | USB_REQ_DEVICE))
            {
                wStatus = usb_status;
            }
            send(&wStatus, sizeof(wStatus));
            break;

        case USB_REQ_CLEAR_FEATURE:
            LOG("CLR FEA");
            if ((request_type == (USB_REQ_HOSTTODEVICE | USB_REQ_STANDARD | USB_REQ_DEVICE)) &&
                (wValue == USB_DEVICE_REMOTE_WAKEUP))
                usb_status &= ~USB_FEATURE_REMOTE_WAKEUP_ENABLED;

            if (request_type == (USB_REQ_HOSTTODEVICE | USB_REQ_STANDARD | USB_REQ_ENDPOINT))
            {
                for (CodalUSBInterface *iface = interfaces; iface; iface = iface->next)
                {
                    if (iface->in && iface->in->ep == (setup.wIndex & 0x7f))
                        iface->in->clearStall();
                    else if (iface->out && iface->out->ep == (setup.wIndex & 0x7f))
                        iface->out->clearStall();
                }
            }
            sendzlp();
            break;
        case USB_REQ_SET_FEATURE:
            LOG("SET FEA");
            if ((request_type == (USB_REQ_HOSTTODEVICE | USB_REQ_STANDARD | USB_REQ_DEVICE)) &&
                (wValue == USB_DEVICE_REMOTE_WAKEUP))
                usb_status |= USB_FEATURE_REMOTE_WAKEUP_ENABLED;
            sendzlp();
            break;
        case USB_REQ_SET_ADDRESS:
            LOG("SET ADDR");
            usb_set_address_pre(wValue);
            sendzlp();
            usb_set_address(wValue);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            LOG("GET DESC");
            status = sendDescriptors(setup);
            break;
        case USB_REQ_SET_DESCRIPTOR:
            LOG("SET DESC");
            stall();
            break;
        case USB_REQ_GET_CONFIGURATION:
            LOG("GET CONF");
            wStatus = 1;
            send(&wStatus, 1);
            break;

        case USB_REQ_SET_CONFIGURATION:
            LOG("SET CONF");
            if (USB_REQ_DEVICE == (request_type & USB_REQ_DESTINATION))
            {
                usb_initialised = setup.wValueL;
                sendzlp();
            }
            else
                status = DEVICE_NOT_SUPPORTED;
            break;
        }
    }
#if CONFIG_ENABLED(DEVICE_WEBUSB)
    else if ((request_type & USB_REQ_TYPE) == USB_REQ_VENDOR)
    {
        switch (setup.bRequest)
        {
        case VENDOR_MS20:
            if (numWebUSBInterfaces == 0)
            {
                status = DEVICE_NOT_SUPPORTED;
            }
            else
            {
                uint8_t buf[WINUSB_SIZE()];
                memcpy(buf, msOS20Descriptor, sizeof(msOS20Descriptor));
                buf[8] = WINUSB_SIZE();
                buf[9] = WINUSB_SIZE() >> 8;
                uint32_t ptr = sizeof(msOS20Descriptor);

                for (CodalUSBInterface *iface = interfaces; iface; iface = iface->next)
                {
                    if (iface->enableWebUSB())
                    {
                        memcpy(buf + ptr, msOS20FunctionDescriptor,
                               sizeof(msOS20FunctionDescriptor));
                        buf[ptr + 4] = iface->interfaceIdx;
                        ptr += sizeof(msOS20FunctionDescriptor);
                    }
                }

                usb_assert(ptr == sizeof(buf));

                send(buf, sizeof(buf));
            }
            break;

        case VENDOR_WEBUSB:
            // this is the place for the WebUSB landing page, if we ever want to do that
            status = DEVICE_NOT_IMPLEMENTED;
            break;
        }
    }
#endif
    else
    {
        status = interfaceRequest(setup, true);
    }

    if (status < 0)
        stall();

    // sending response clears this - make sure we did
    usb_assert(ctrlIn->wLength == 0);
}

void CodalUSB::interruptHandler()
{
    for (CodalUSBInterface *iface = interfaces; iface; iface = iface->next)
        iface->endpointRequest();
}

void CodalUSB::initEndpoints()
{
    uint8_t endpointCount = 1;
    uint8_t ifaceCount = 0;

    if (ctrlIn)
    {
        delete ctrlIn;
        delete ctrlOut;
    }

    ctrlIn = new UsbEndpointIn(0, USB_EP_TYPE_CONTROL);
    ctrlOut = new UsbEndpointOut(0, USB_EP_TYPE_CONTROL);

#if CONFIG_ENABLED(DEVICE_WEBUSB)
    numWebUSBInterfaces = 0;
#endif

    for (CodalUSBInterface *iface = interfaces; iface; iface = iface->next)
    {
        iface->interfaceIdx = ifaceCount++;

#if CONFIG_ENABLED(DEVICE_WEBUSB)
        if (iface->enableWebUSB())
            numWebUSBInterfaces++;
#endif

        const InterfaceInfo *info = iface->getInterfaceInfo();

        usb_assert(0 <= info->allocateEndpoints && info->allocateEndpoints <= 2);
        usb_assert(info->allocateEndpoints <= info->iface.numEndpoints &&
                   info->iface.numEndpoints <= 2);

        if (iface->in)
        {
            delete iface->in;
            iface->in = NULL;
        }

        if (iface->out)
        {
            delete iface->out;
            iface->out = NULL;
        }

        uint8_t numep = NUM_ENDPOINTS(info->allocateEndpoints);

        if (info->iface.numEndpoints > 0)
        {
            iface->in = new UsbEndpointIn(endpointCount, info->epIn.attr);
            if (info->iface.numEndpoints > 1)
            {
                iface->out = new UsbEndpointOut(endpointCount + (numep - 1), info->epIn.attr);
            }
        }

        endpointCount += numep;
    }

    usb_assert(endpointsUsed == endpointCount);
}

int CodalUSB::start()
{
    if (--startDelayCount > 0)
    {
        DMESG("USB start delayed");
        return DEVICE_OK;
    }

    DMESG("USB start");

    if (DEVICE_USB_ENDPOINTS == 0)
        return DEVICE_NOT_SUPPORTED;

    if (usb_configured)
        return DEVICE_OK;

    usb_configured = 1;

    usb_configure(endpointsUsed);

    return DEVICE_OK;
}

void usb_panic(int lineNumber)
{
    DMESG("USB assertion failed: line %d", lineNumber);
    target_panic(DEVICE_USB_ERROR);
}

#endif
