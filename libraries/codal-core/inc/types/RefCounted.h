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

#ifndef REF_COUNTED_H
#define REF_COUNTED_H

#include "CodalConfig.h"
#include "CodalDevice.h"

namespace codal
{
    /**
      * Base class for payload for ref-counted objects. Used by ManagedString and DeviceImage.
      * There is no constructor, as this struct is typically malloc()ed.
      */
    struct RefCounted
    {
    public:
        /**
          * The high 15 bits hold the number of outstanding references. The lowest bit is always 1
          * to make sure it doesn't look like C++ vtable.
          * Should never be even or one (object should be deleted then).
          * When it's set to 0xffff, it means the object sits in flash and should not be counted.
          */
        uint16_t refCount;

    #if CONFIG_ENABLED(DEVICE_TAG)
        uint16_t tag;
    #endif

        /**
          * Increment reference count.
          */
        void incr();

        /**
            * Decrement reference count.
            */
        void decr();

        /**
          * Initializes for one outstanding reference.
          */
        void init();

        /**
          * Releases the current instance.
          */
        void destroy();

        /**
          * Checks if the object resides in flash memory.
          *
          * @return true if the object resides in flash memory, false otherwise.
          */
        bool isReadOnly();
    };


    #if CONFIG_ENABLED(DEVICE_TAG)
    // Note that there might be binary dependencies on these values (and layout of
    // RefCounted and derived classes), so the existing ones are best left unchanged.
    #define REF_TAG_STRING 1
    #define REF_TAG_BUFFER 2
    #define REF_TAG_IMAGE 3
    #define REF_TAG_USER 32

    #define REF_COUNTED_DEF_EMPTY(...)                                                                 \
        static const uint16_t emptyData[]                                                              \
            __attribute__((aligned(4))) = {0xffff, REF_TAG, __VA_ARGS__};
    #define REF_COUNTED_INIT(ptr)                                                                      \
        ptr->init();                                                                                   \
        ptr->tag = REF_TAG
    #else
    #define REF_COUNTED_DEF_EMPTY(className, ...)                                                      \
        static const uint16_t emptyData[] __attribute__((aligned(4))) = {0xffff, __VA_ARGS__};
    #define REF_COUNTED_INIT(ptr) ptr->init()
    #endif
}


#endif
