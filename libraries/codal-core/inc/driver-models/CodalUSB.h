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

#ifndef CODAL_USB_H
#define CODAL_USB_H

#include "CodalConfig.h"

#if CONFIG_ENABLED(DEVICE_USB)

#include <stdint.h>
#include "ErrorNo.h"

#define USB_CONFIG_POWERED_MASK 0x40
#define USB_CONFIG_BUS_POWERED 0x80
#define USB_CONFIG_SELF_POWERED 0xC0
#define USB_CONFIG_REMOTE_WAKEUP 0x20

#define USB_DEVICE_DESCRIPTOR_TYPE 1
#define USB_CONFIGURATION_DESCRIPTOR_TYPE 2
#define USB_STRING_DESCRIPTOR_TYPE 3
#define USB_INTERFACE_DESCRIPTOR_TYPE 4
#define USB_ENDPOINT_DESCRIPTOR_TYPE 5
#define USB_BOS_DESCRIPTOR_TYPE 15

#define USB_REQ_HOSTTODEVICE 0x00
#define USB_REQ_DEVICETOHOST 0x80
#define USB_REQ_DIRECTION 0x80

#define USB_REQ_STANDARD 0x00
#define USB_REQ_CLASS 0x20
#define USB_REQ_VENDOR 0x40
#define USB_REQ_TYPE 0x60

#define USB_REQ_DESTINATION 0x1F
#define USB_REQ_DEVICE 0x00
#define USB_REQ_INTERFACE 0x01
#define USB_REQ_ENDPOINT 0x02
#define USB_REQ_OTHER 0x03

#define USB_REQ_GET_STATUS 0x00
#define USB_REQ_CLEAR_FEATURE 0x01
#define USB_REQ_SET_FEATURE 0x03
#define USB_REQ_SET_ADDRESS 5
#define USB_REQ_GET_DESCRIPTOR 6
#define USB_REQ_SET_DESCRIPTOR 7
#define USB_REQ_GET_CONFIGURATION 8
#define USB_REQ_SET_CONFIGURATION 9
#define USB_REQ_GET_INTERFACE 10
#define USB_REQ_SET_INTERFACE 11
#define USB_REQ_SYNCH_FRAME 12

#define USB_DEVICE_REMOTE_WAKEUP 1
#define USB_FEATURE_SELFPOWERED_ENABLED (1 << 0)
#define USB_FEATURE_REMOTE_WAKEUP_ENABLED (1 << 1)

enum usb_ep_type
{
    USB_EP_TYPE_CONTROL = 0x00,
    USB_EP_TYPE_ISOCHRONOUS = 0x01,
    USB_EP_TYPE_BULK = 0x02,
    USB_EP_TYPE_INTERRUPT = 0x03,
};

//    Device
typedef struct
{
    uint8_t len;         // 18
    uint8_t dtype;       // 1 USB_DEVICE_DESCRIPTOR_TYPE
    uint16_t usbVersion; // 0x200 or 0x210
    uint8_t deviceClass;
    uint8_t deviceSubClass;
    uint8_t deviceProtocol;
    uint8_t packetSize0; // Packet 0
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t deviceVersion; // 0x100
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __attribute__((packed)) DeviceDescriptor;

//    Config
typedef struct
{
    uint8_t len;   // 9
    uint8_t dtype; // 2
    uint16_t clen; // total length
    uint8_t numInterfaces;
    uint8_t config;
    uint8_t iconfig;
    uint8_t attributes;
    uint8_t maxPower;
} __attribute__((packed)) ConfigDescriptor;

//    Interface
typedef struct
{
    uint8_t len;
    uint8_t dtype;
    uint8_t number;
    uint8_t alternate;
    uint8_t numEndpoints;
    uint8_t interfaceClass;
    uint8_t interfaceSubClass;
    uint8_t protocol;
    uint8_t iInterface;
} __attribute__((packed)) InterfaceDescriptor;

typedef struct
{
    uint8_t numEndpoints;
    uint8_t interfaceClass;
    uint8_t interfaceSubClass;
    uint8_t protocol;
    uint8_t iInterfaceString;
    uint8_t alternate;
} InterfaceDescriptorInfo;

typedef struct
{
    uint8_t attr;
    uint8_t interval;
} EndpointDescriptorInfo;

typedef struct
{
    const void *supplementalDescriptor;
    uint16_t supplementalDescriptorSize;
    // For interrupt endpoints, this will be 1, even if iface.numEndpoints is 2.
    // This is because a single USB endpoint address will be used for both.
    uint8_t allocateEndpoints;
    InterfaceDescriptorInfo iface;
    EndpointDescriptorInfo epIn;
    EndpointDescriptorInfo epOut;
} InterfaceInfo;

//    Endpoint
typedef struct
{
    uint8_t len;
    uint8_t dtype;
    uint8_t addr;
    uint8_t attr;
    uint16_t packetSize;
    uint8_t interval;
} __attribute__((packed)) EndpointDescriptor;

#define EP_DESC2(tp, interval)                                                                     \
    {7, 5, 0x80, tp, USB_MAX_PKT_SIZE, interval}, { 7, 5, 0x00, tp, USB_MAX_PKT_SIZE, interval }

typedef struct
{
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint8_t wValueL;
    uint8_t wValueH;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__((packed)) USBSetup;

//    String
typedef struct
{
    uint8_t len;
    uint8_t type;
    // some reasonable size; it gets stack allocated
    uint16_t data[64];
} __attribute__((packed)) StringDescriptor;

#pragma GCC diagnostic ignored "-Wunused-parameter"

class UsbEndpointIn
{
    uint8_t buf[USB_MAX_PKT_SIZE];
public:
    volatile uint32_t userdata;
    uint8_t ep;
    uint8_t flags;
    uint16_t wLength;
    int stall();
    int clearStall();
    int reset();
    int write(const void *buf, int length);

    UsbEndpointIn(uint8_t idx, uint8_t type, uint8_t size = USB_MAX_PKT_SIZE);
};

class UsbEndpointOut
{
    uint8_t buf[USB_MAX_PKT_SIZE];

public:
    volatile uint32_t userdata;
    uint8_t ep;
    int stall();
    int reset();
    int clearStall();
    int read(void *buf, int maxlength); // up to packet size
    // when IRQ disabled, endpointRequest() callback will not be called (generally)
    int disableIRQ();
    int enableIRQ();
    void startRead();

    UsbEndpointOut(uint8_t idx, uint8_t type, uint8_t size = USB_MAX_PKT_SIZE);
};

void usb_configure(uint8_t numEndpoints);
void usb_set_address(uint16_t wValue);
void usb_set_address_pre(uint16_t wValue);

class CodalUSBInterface
{
public:
    uint8_t interfaceIdx;
    UsbEndpointIn *in;
    UsbEndpointOut *out;
    CodalUSBInterface *next;

    CodalUSBInterface()
    {
        in = 0;
        out = 0;
        interfaceIdx = 0;
        next = NULL;
    }

    virtual int classRequest(UsbEndpointIn &ctrl, USBSetup &setup) { return DEVICE_NOT_SUPPORTED; }
    // standard request to interface (eg USB_REQ_GET_DESCRIPTOR)
    virtual int stdRequest(UsbEndpointIn &ctrl, USBSetup &setup) { return DEVICE_NOT_SUPPORTED; }
    virtual int endpointRequest() { return DEVICE_NOT_SUPPORTED; }
    virtual const InterfaceInfo *getInterfaceInfo() { return NULL; }
    void fillInterfaceInfo(InterfaceDescriptor *desc);
    virtual bool enableWebUSB() { return false; }
};

class CodalDummyUSBInterface : public CodalUSBInterface {
  public:
    virtual const InterfaceInfo *getInterfaceInfo();
};

class CodalUSB
{
    uint8_t endpointsUsed;
    uint8_t startDelayCount;
    uint8_t numWebUSBInterfaces;

    int sendConfig();
    int sendDescriptors(USBSetup &setup);
    int interfaceRequest(USBSetup &setup, bool isClass);

public:
    static CodalUSB *usbInstance;
    CodalUSBInterface *interfaces;

    // initialized by constructor, can be overriden before start()
    uint8_t numStringDescriptors;
    uint8_t maxPower;
    const char **stringDescriptors;
    const DeviceDescriptor *deviceDescriptor;

    UsbEndpointIn *ctrlIn;
    UsbEndpointOut *ctrlOut;

    CodalUSB();

    int add(CodalUSBInterface &interface);

    int isInitialised();

    CodalUSB *getInstance();

    int start();
    // an interface can call it and, at some later point, call start()
    void delayStart() { startDelayCount++; }

    // Called from USB.cpp
    void setupRequest(USBSetup &setup);
    void interruptHandler();
    void initEndpoints();
};

void usb_panic(int lineNumber);

#define usb_assert(cond)                                                                           \
    if (!(cond))                                                                                   \
    usb_panic(__LINE__)

#endif

#endif
