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

#ifndef DEVICE_USBMSC_H
#define DEVICE_USBMSC_H

#include "CodalUSB.h"
#include "Event.h"

#if CONFIG_ENABLED(DEVICE_USB)

namespace codal
{

struct MSCState;

class USBMSC : public CodalUSBInterface
{
    struct MSCState *state;
    uint32_t blockAddr;
    uint16_t blockCount;
    bool failed;
    bool listen;
    bool disableIRQ;

    bool writePadded(const void *ptr, int dataSize, int allocSize = -1);
    void writeHandler(Event);
    void readHandler(Event);

    int handeSCSICommand();
    int sendResponse(bool ok);
    void fail();

    bool cmdInquiry();
    bool cmdRequest_Sense();
    bool cmdRead_Capacity_10();
    bool cmdSend_Diagnostic();
    void cmdReadWrite_10(bool isRead);
    bool cmdModeSense(bool is10);
    bool cmdReadFormatCapacity();

public:
    USBMSC();
    virtual int endpointRequest();
    virtual int classRequest(UsbEndpointIn &ctrl, USBSetup &setup);
    virtual const InterfaceInfo *getInterfaceInfo();

    virtual int totalLUNs() { return 1; }
    virtual bool storageOK() { return true; }
    virtual bool isReadOnly() { return false; }
    virtual uint32_t getCapacity() { return 8 * 1024 * 2; } // 8M

    void writeBulk(const void *ptr, int dataSize);
    void readBulk(void *ptr, int dataSize);
    void finishReadWrite();
    int currLUN();
    uint32_t cbwTag();

    virtual void readBlocks(int blockAddr, int numBlocks) = 0;
    virtual void writeBlocks(int blockAddr, int numBlocks) = 0;
};
}

#endif

#endif
