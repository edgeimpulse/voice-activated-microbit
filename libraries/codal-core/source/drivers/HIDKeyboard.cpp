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

#include "HIDKeyboard.h"
#include "HID.h"
#include "AsciiKeyMap.h"
#include "CodalDmesg.h"

#if CONFIG_ENABLED(DEVICE_USB)

using namespace codal;

#define HID_KEYBOARD_KEY_OFF 0x00

//this descriptor must be stored in RAM
static char hidKeyboardDescriptor[] = {
    0x05, 0x01,                         // Usage Page (Generic Desktop)
    0x09, 0x06,                         // Usage (Keyboard)
    0xA1, 0x01,                         // Collection (Application)
    0x85, HID_KEYBOARD_REPORT_GENERIC,  //   Report ID (1)
    0x05, 0x07,                         //     Usage Page (Key Codes)
    0x19, 0xe0,                         //     Usage Minimum (224)
    0x29, 0xe7,                         //     Usage Maximum (231)
    0x15, 0x00,                         //     Logical Minimum (0)
    0x25, 0x01,                         //     Logical Maximum (1)
    0x75, 0x01,                         //     Report Size (1)
    0x95, 0x08,                         //     Report Count (8)
    0x81, 0x02,                         //     Input (Data, Variable, Absolute)

    0x95, 0x01,                         //     Report Count (1)
    0x75, 0x08,                         //     Report Size (8)
    0x81, 0x01,                         //     Input (Constant) reserved byte(1)

    0x95, 0x05,                         //     Report Count (5)
    0x75, 0x01,                         //     Report Size (1)
    0x05, 0x08,                         //     Usage Page (Page# for LEDs)
    0x19, 0x01,                         //     Usage Minimum (1)
    0x29, 0x05,                         //     Usage Maximum (5)
    0x91, 0x02,                         //     Output (Data, Variable, Absolute), Led report
    0x95, 0x01,                         //     Report Count (1)
    0x75, 0x03,                         //     Report Size (3)
    0x91, 0x01,                         //     Output (Data, Variable, Absolute), Led report padding

    0x95, 0x06,                         //     Report Count (6)
    0x75, 0x08,                         //     Report Size (8)
    0x15, 0x00,                         //     Logical Minimum (0)
    0x25, 0x65,                         //     Logical Maximum (101)
    0x05, 0x07,                         //     Usage Page (Key codes)
    0x19, 0x00,                         //     Usage Minimum (0)
    0x29, 0x65,                         //     Usage Maximum (101)
    0x81, 0x00,                         //     Input (Data, Array) Key array(6 bytes)


    0x09, 0x05,                         //     Usage (Vendor Defined)
    0x15, 0x00,                         //     Logical Minimum (0)
    0x26, 0xFF, 0x00,                   //     Logical Maximum (255)
    0x75, 0x08,                         //     Report Count (2)
    0x95, 0x02,                         //     Report Size (8 bit)
    0xB1, 0x02,                         //     Feature (Data, Variable, Absolute)

    0xC0,                                // End Collection (Application)

    0x05, 0x0c,                         // Usage Page (Consumer Devices)
    0x09, 0x01,                         // Usage (Consumer Control)
    0xa1, 0x01,                         // Collection (Application)
    0x85, HID_KEYBOARD_REPORT_CONSUMER, // Report ID (2)
    0x15, 0x00,                         // Logical Minimum (0)
    0x25, 0x01,                         // Logical Maximum (1)
    0x75, 0x01,                         // Report Size (1)
    0x95, 0x10,                         // Report Count (16)
    0x09, 0xe2,                         // Usage (Mute) 0x01
    0x09, 0xe9,                         // Usage (Volume Up) 0x02
    0x09, 0xea,                         // Usage (Volume Down) 0x03
    0x09, 0xcd,                         // Usage (Play/Pause) 0x04
    0x09, 0xb7,                         // Usage (Stop) 0x05
    0x09, 0xb6,                         // Usage (Scan Previous Track) 0x06
    0x09, 0xb5,                         // Usage (Scan Next Track) 0x07
    0x0a, 0x8a, 0x01,                   // Usage (Mail) 0x08
    0x0a, 0x92, 0x01,                   // Usage (Calculator) 0x09
    0x0a, 0x21, 0x02,                   // Usage (www search) 0x0a
    0x0a, 0x23, 0x02,                   // Usage (www home) 0x0b
    0x0a, 0x2a, 0x02,                   // Usage (www favorites) 0x0c
    0x0a, 0x27, 0x02,                   // Usage (www refresh) 0x0d
    0x0a, 0x26, 0x02,                   // Usage (www stop) 0x0e
    0x0a, 0x25, 0x02,                   // Usage (www forward) 0x0f
    0x0a, 0x24, 0x02,                   // Usage (www back) 0x10
    0x81, 0x62,                         // Input (Data,Var,Abs,NPrf,Null)
    0xc0,                               // End Collection

};

static const HIDReportDescriptor reportDesc = {
    9,
    0x21,                  // HID
    0x101,                 // hidbcd 1.01
    0x00,                  // country code
    0x01,                  // num desc
    0x22,                  // report desc type
    sizeof(hidKeyboardDescriptor),
};

static const InterfaceInfo ifaceInfo = {
    &reportDesc,
    sizeof(reportDesc),
    1,
    {
        1,    // numEndpoints
        0x03, /// class code - HID
        0x01, // subclass (boot interface)
        0x01, // protocol (keyboard)
        0x00, //
        0x00, //
    },
    {USB_EP_TYPE_INTERRUPT, 1},
    {USB_EP_TYPE_INTERRUPT, 1},
};

static KeyMap* currentMap = NULL;

extern AsciiKeyMap asciiKeyMap;


/**
  * initialises the report arrays for this USBHID instance.
  */
void USBHIDKeyboard::initReports()
{
    reports[HID_KEYBOARD_REPORT_GENERIC].reportID = HID_KEYBOARD_REPORT_GENERIC;
    reports[HID_KEYBOARD_REPORT_GENERIC].keyState = keyStateGeneric;
    reports[HID_KEYBOARD_REPORT_GENERIC].reportSize = HID_KEYBOARD_KEYSTATE_SIZE_GENERIC;
    reports[HID_KEYBOARD_REPORT_GENERIC].keyPressedCount = 0;

    reports[HID_KEYBOARD_REPORT_CONSUMER].reportID = HID_KEYBOARD_REPORT_CONSUMER;
    reports[HID_KEYBOARD_REPORT_CONSUMER].keyState = keyStateConsumer;
    reports[HID_KEYBOARD_REPORT_CONSUMER].reportSize = HID_KEYBOARD_KEYSTATE_SIZE_CONSUMER;
    reports[HID_KEYBOARD_REPORT_CONSUMER].keyPressedCount = 0;

    memset(keyStateGeneric, 0, HID_KEYBOARD_KEYSTATE_SIZE_GENERIC);
    memset(keyStateConsumer, 0, HID_KEYBOARD_KEYSTATE_SIZE_CONSUMER);
}

/**
  * Default constructor for a USBHIDKeyboard instance, sets the KeyMap to an ASCII default .
  */
USBHIDKeyboard::USBHIDKeyboard()
{
    initReports();
    setKeyMap(asciiKeyMap);
}

/**
  * Constructor for a USBHIDKeyboard instance, sets the KeyMap to the given keymap.
  *
  * @param k The KeyMap to use.
  */
USBHIDKeyboard::USBHIDKeyboard(KeyMap& m)
{
    initReports();
    setKeyMap(m);
}

/**
  * Sets the KeyMap for this USBHIDKeyboard instance
  *
  * @param map The KeyMap to use.
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::setKeyMap(KeyMap& m)
{
    currentMap = &m;
    return DEVICE_OK;
}

/**
  * Writes the given report out over USB.
  *
  * @param report A pointer to the report to copy to USB
  */
int USBHIDKeyboard::updateReport(HIDKeyboardReport* report)
{
    if(report == NULL)
        return DEVICE_INVALID_PARAMETER;

    uint8_t reportBuf[report->reportSize + 1] = {report->reportID};
    memcpy(reportBuf + 1, report->keyState, report->reportSize);

    return in->write(reportBuf, sizeof(reportBuf));
}


/**
  * sets the media key buffer to the given Key, without affecting the state of other media keys.
  *
  * @param k a valid media key
  *
  * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if an incorrect key is passed.
  */
int USBHIDKeyboard::mediaKeyPress(Key k, KeyActionType action)
{
    int status = DEVICE_OK;
    HIDKeyboardReport *report = &reports[HID_KEYBOARD_REPORT_CONSUMER];

    uint16_t* keyState = (uint16_t *)report->keyState;
    uint16_t currentModifier = *keyState;

    if (action == ReleaseKey)
        *keyState &= ~k.bit.code;
    else
         *keyState |= k.bit.code;

    // update only when required.
    if(*keyState != currentModifier)
        status = updateReport(report);

     //we could not make the change. Revert
    if(status != DEVICE_OK)
    {
        *keyState = currentModifier;
        return status;
    }

    return status;
}

/**
  * sets the keyboard modifier buffer to the given Key, without affecting the state of other keys.
  *
  * @param k a valid modifier key
  *
  * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if an incorrect key is passed.
  */
int USBHIDKeyboard::modifierKeyPress(Key k, KeyActionType action)
{
    int status = DEVICE_OK;
    uint8_t currentModifier;

    HIDKeyboardReport *report = &reports[HID_KEYBOARD_REPORT_GENERIC];
    currentModifier = *report->keyState;

    if (action == ReleaseKey)
        *report->keyState &= ~k.bit.code;
    else
        *report->keyState = currentModifier | k.bit.code;

    // update only when required.
    if(*report->keyState != currentModifier)
        status = updateReport(report);

     //we could not make the change. Revert
    if(status != DEVICE_OK)
    {
        *report->keyState = currentModifier;
        return status;
    }

    return DEVICE_OK;
}


/**
  * sets one keyboard key buffer slot to the given Key.
  *
  * @param k a valid modifier key
  *
  * @return DEVICE_OK on success
  */
int USBHIDKeyboard::standardKeyPress(Key k, KeyActionType action)
{
    int status = DEVICE_OK;
    HIDKeyboardReport *report = &reports[HID_KEYBOARD_REPORT_GENERIC];

    if(report->keyPressedCount == 0 && action == ReleaseKey)
        return DEVICE_INVALID_PARAMETER;

    if(report->keyPressedCount == report->reportSize && action == PressKey)
        return DEVICE_NO_RESOURCES;

    // firstly iterate through the array to determine if we are raising a key
    int existingIndex = -1;
    int firstSpareSlot = -1;

    for(int i = HID_KEYBOARD_MODIFIER_OFFSET; i < report->reportSize; i++)
    {
        // we can break here, we don't need a spare slot, we are raising...
        if(report->keyState[i] == k.bit.code)
        {
            existingIndex = i;
            break;
        }

        if(report->keyState[i] == HID_KEYBOARD_KEY_OFF && firstSpareSlot == -1)
            firstSpareSlot = i;
    }

    // if found and correct action requested, perform action
    if(existingIndex > -1 && action == ReleaseKey)
        report->keyState[existingIndex] = HID_KEYBOARD_KEY_OFF;
    else if(firstSpareSlot > -1 && action == PressKey)
        report->keyState[firstSpareSlot] = k.bit.code;

    status = updateReport(report);

    // update counts...
    if(status == DEVICE_OK)
    {
        if(action == ReleaseKey)
            report->keyPressedCount--;
        else if(action == PressKey)
            report->keyPressedCount++;
    }
    else
        // revert we could not make the change
        report->keyState[firstSpareSlot] = HID_KEYBOARD_KEY_OFF;

    return status;
}

/**
  * Releases the given Key.
  *
  * @param k A valid Key
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::keyUp(Key k)
{
    int status = DEVICE_OK;

    if(k.bit.isModifier)
        status = modifierKeyPress(k, ReleaseKey);
    else if(k.bit.isMedia)
        status = mediaKeyPress(k, ReleaseKey);
    else
        status = standardKeyPress(k, ReleaseKey);

    fiber_sleep(HID_KEYBOARD_DELAY_DEFAULT);

    return status;
}

/**
  * Press the given Key.
  *
  * @param k A valid Key
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::keyDown(Key k)
{
    int status = DEVICE_OK;

    if(k.bit.isModifier)
        status = modifierKeyPress(k, PressKey);
    else if(k.bit.isMedia)
        status = mediaKeyPress(k, PressKey);
    else
        status = standardKeyPress(k, PressKey);

    fiber_sleep(HID_KEYBOARD_DELAY_DEFAULT);

    return status;
}

/**
  * Releases the given Key.
  *
  * @param k A valid MediaKey
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::keyUp(MediaKey k)
{
    return keyUp(currentMap->getMediaKey(k));
}

/**
  * Press the given Key.
  *
  * @param k A valid MediaKey
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::keyDown(MediaKey k)
{
    return keyDown(currentMap->getMediaKey(k));
}

/**
  * Releases the given Key.
  *
  * @param k A valid FunctionKey
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::keyUp(FunctionKey k)
{
    return keyUp(currentMap->getFunctionKey(k));
}

/**
  * Press the given Key.
  *
  * @param k A valid FunctionKey
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::keyDown(FunctionKey k)
{
    return keyDown(currentMap->getFunctionKey(k));
}

/**
  * Release the key corresponding to the given character.
  *
  * @param c A valid character
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::keyUp(uint16_t c)
{
    const KeySequence* seq =  currentMap->mapCharacter(c);
    int status = DEVICE_OK;

    if(seq == NULL)
        return DEVICE_INVALID_PARAMETER;

    for(int i = 0; i < seq->length; i++)
    {
        Key k = seq->seq[i];
        status = keyUp(k);

        if(status != DEVICE_OK)
            break;
    }

    return status;
}

/**
  * Press the key corresponding to the given character.
  *
  * @param c A valid character
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::keyDown(uint16_t c)
{
    const KeySequence* seq =  currentMap->mapCharacter(c);
    int status = DEVICE_OK;

    if(seq == NULL)
        return DEVICE_INVALID_PARAMETER;

    for(int i = 0; i < seq->length; i++)
    {
        Key k = seq->seq[i];
        status = keyDown(k);

        if(status != DEVICE_OK)
            break;
    }

    return status;
}

/**
  * Presses and releases the given Key.
  *
  * @param k A valid Key
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::press(Key k)
{
    keyDown(k);
    keyUp(k);
    return DEVICE_OK;
}

/**
  * Press and releases the given Key.
  *
  * @param k A valid MediaKey
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::press(MediaKey k)
{
    return press(currentMap->getMediaKey(k));
}

/**
  * Press and releases the given Key.
  *
  * @param k A valid FunctionKey
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::press(FunctionKey k)
{
    return press(currentMap->getFunctionKey(k));
}

/**
  * Press and releases the Key corresponding to the given character.
  *
  * @param k A valid character
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::press(uint16_t c)
{
    int status = DEVICE_OK;

    // status doesn't really matter here - if one fails the other likely will.
    status = keyDown(c);
    status = keyUp(c);

    return status;
}

/**
  * Releases ALL keys on the keyboard (including Media keys)
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::flush()
{
    int status = DEVICE_OK;

    HIDKeyboardReport *report = &reports[HID_KEYBOARD_REPORT_GENERIC];
    memset(report->keyState, 0, report->reportSize);
    status = updateReport(report);

    report->keyPressedCount = 0;

    if(status != DEVICE_OK)
        return status;

    report = &reports[HID_KEYBOARD_REPORT_CONSUMER];
    memset(report->keyState, 0, report->reportSize);
    status = updateReport(report);

    report->keyPressedCount = 0;


    return status;
}

/**
  * Type a sequence of keys
  *
  * @param seq A valid pointer to a KeySequence containing multiple keys. See ASCIIKeyMap.cpp for example usage.
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::type(const KeySequence *seq)
{
    if(seq == NULL)
        return DEVICE_INVALID_PARAMETER;

    //send each keystroke in the sequence
    for(int i = 0; i < seq->length; i++)
    {
        Key k = seq->seq[i];

        if(k.bit.allKeysUp)
            flush();

        if(k.bit.isKeyDown)
            keyDown(k);
        else
            keyUp(k);
    }

    //all keys up is implicit at the end of each sequence
    flush();
    fiber_sleep(HID_KEYBOARD_DELAY_DEFAULT);

    return DEVICE_OK;
}

/**
  * Type a sequence of characters
  *
  * @param s A valid pointer to a char array
  *
  * @param len The length of s.
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::type(const char* s, uint32_t len)
{
    int status;

    for(uint32_t i = 0; i < len; i++)
    {
        if((status = type(currentMap->mapCharacter(s[i]))) != DEVICE_OK)
           return status;
    }

    return DEVICE_OK;
}

/**
  * Type a sequence of characters
  *
  * @param s A ManagedString instance containing the characters to type.
  *
  * @return DEVICE_OK on success.
  */
int USBHIDKeyboard::type(ManagedString s)
{
    return type(s.toCharArray(), s.length());
}

int USBHIDKeyboard::stdRequest(UsbEndpointIn &ctrl, USBSetup &setup)
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
            return ctrl.write(hidKeyboardDescriptor, sizeof(hidKeyboardDescriptor));
        }
    }
    return DEVICE_NOT_SUPPORTED;
}

const InterfaceInfo *USBHIDKeyboard::getInterfaceInfo()
{
    return &ifaceInfo;
}


#endif