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

#ifndef DEVICE_HID_KEYBOARD_H
#define DEVICE_HID_KEYBOARD_H

#include "HID.h"
#include "KeyMap.h"
#include "ManagedString.h"

#if CONFIG_ENABLED(DEVICE_USB)

//report 0 is empty
#define HID_KEYBOARD_NUM_REPORTS 3

#define HID_KEYBOARD_REPORT_GENERIC 0x01
#define HID_KEYBOARD_REPORT_CONSUMER 0x02
#define HID_KEYBOARD_KEYSTATE_SIZE_GENERIC 0x08
#define HID_KEYBOARD_KEYSTATE_SIZE_CONSUMER 0x02
#define HID_KEYBOARD_MODIFIER_OFFSET 2

#define HID_KEYBOARD_DELAY_DEFAULT 10

namespace codal
{
    enum KeyActionType
    {
        PressKey,
        ReleaseKey
    };

    typedef struct {
        uint8_t reportID;
        uint8_t *keyState;
        uint8_t reportSize;
        uint8_t keyPressedCount;
    } HIDKeyboardReport;

    class USBHIDKeyboard : public USBHID
    {
        uint8_t keyStateGeneric[HID_KEYBOARD_KEYSTATE_SIZE_GENERIC];
        uint8_t keyStateConsumer[HID_KEYBOARD_KEYSTATE_SIZE_CONSUMER];

        /**
          * Writes the given report out over USB.
          *
          * @param report A pointer to the report to copy to USB
          */
        int updateReport(HIDKeyboardReport* report);

        /**
          * sets the media key buffer to the given Key, without affecting the state of other media keys.
          *
          * @param k a valid media key
          *
          * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if an incorrect key is passed.
          */
        int mediaKeyPress(Key k, KeyActionType action);

        /**
          * sets the keyboard modifier buffer to the given Key, without affecting the state of other keys.
          *
          * @param k a valid modifier key
          *
          * @return DEVICE_OK on success, DEVICE_INVALID_PARAMETER if an incorrect key is passed.
          */
        int modifierKeyPress(Key k, KeyActionType action);

        /**
          * sets one keyboard key buffer slot to the given Key.
          *
          * @param k a valid modifier key
          *
          * @return DEVICE_OK on success
          */
        int standardKeyPress(Key k, KeyActionType action);

        /**
          * initialises the report arrays for this USBHID instance.
          */
        void initReports();

    public:

        /**
          * Default constructor for a USBHIDKeyboard instance, sets the KeyMap to an ASCII default .
          */
        USBHIDKeyboard();

        /**
          * Constructor for a USBHIDKeyboard instance, sets the KeyMap to the given keymap.
          *
          * @param k The KeyMap to use.
          */
        USBHIDKeyboard(KeyMap& k);

        /**
          * Sets the KeyMap for this USBHIDKeyboard instance
          *
          * @param map The KeyMap to use.
          *
          * @return DEVICE_OK on success.
          */
        int setKeyMap(KeyMap& map);

        /**
          * Releases the given Key.
          *
          * @param k A valid Key
          *
          * @return DEVICE_OK on success.
          */
        int keyUp(Key k);

        /**
          * Releases the given Key.
          *
          * @param k A valid MediaKey
          *
          * @return DEVICE_OK on success.
          */
        int keyUp(MediaKey k);

        /**
          * Releases the given Key.
          *
          * @param k A valid FunctionKey
          *
          * @return DEVICE_OK on success.
          */
        int keyUp(FunctionKey k);

        /**
          * Release the key corresponding to the given character.
          *
          * @param c A valid character
          *
          * @return DEVICE_OK on success.
          */
        int keyUp(uint16_t c);

        /**
          * Press the given Key.
          *
          * @param k A valid Key
          *
          * @return DEVICE_OK on success.
          */
        int keyDown(Key k);

        /**
          * Press the given Key.
          *
          * @param k A valid MediaKey
          *
          * @return DEVICE_OK on success.
          */
        int keyDown(MediaKey k);

        /**
          * Press the given Key.
          *
          * @param k A valid FunctionKey
          *
          * @return DEVICE_OK on success.
          */
        int keyDown(FunctionKey k);

        /**
          * Press the key corresponding to the given character.
          *
          * @param c A valid character
          *
          * @return DEVICE_OK on success.
          */
        int keyDown(uint16_t c);

        /**
          * Presses and releases the given Key.
          *
          * @param k A valid Key
          *
          * @return DEVICE_OK on success.
          */
        int press(Key k);

        /**
          * Press and releases the given Key.
          *
          * @param k A valid MediaKey
          *
          * @return DEVICE_OK on success.
          */
        int press(MediaKey k);

        /**
          * Press and releases the given Key.
          *
          * @param k A valid FunctionKey
          *
          * @return DEVICE_OK on success.
          */
        int press(FunctionKey k);

        /**
          * Press and releases the Key corresponding to the given character.
          *
          * @param k A valid character
          *
          * @return DEVICE_OK on success.
          */
        int press(uint16_t c);

        /**
          * Releases ALL keys on the keyboard (including Media keys)
          *
          * @return DEVICE_OK on success.
          */
        int flush();

        /**
          * Type a sequence of keys
          *
          * @param seq A valid pointer to a KeySequence containing multiple keys. See ASCIIKeyMap.cpp for example usage.
          *
          * @return DEVICE_OK on success.
          */
        int type(const KeySequence *seq);

        /**
          * Type a sequence of characters
          *
          * @param s A valid pointer to a char array
          *
          * @param len The length of s.
          *
          * @return DEVICE_OK on success.
          */
        int type(const char* s, uint32_t len);

        /**
          * Type a sequence of characters
          *
          * @param s A ManagedString instance containing the characters to type.
          *
          * @return DEVICE_OK on success.
          */
        int type(ManagedString s);

        HIDKeyboardReport reports[HID_KEYBOARD_NUM_REPORTS];

        virtual int stdRequest(UsbEndpointIn &ctrl, USBSetup& setup);
        virtual const InterfaceInfo *getInterfaceInfo();
    };
}


#endif

#endif