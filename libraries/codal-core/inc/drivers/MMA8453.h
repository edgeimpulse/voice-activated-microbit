/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

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

#ifndef CODAL_MMA8453_H
#define CODAL_MMA8453_H

#include "MMA8653.h"

// It's almost like 8653, except for address

#define MMA8453_DEFAULT_ADDR    0x38
#define MMA8453_WHOAMI_VAL      0x3A

namespace codal
{
    /**
     * Class definition for MMA8453 Accelerometer.
     *
     * Represents an implementation of the Freescale MMA8453 3 axis accelerometer
     * Also includes basic data caching and on demand activation.
     */
    class MMA8453 : public MMA8653
    {
        public:

        /**
          * Constructor.
          * Create a software abstraction of an accelerometer.
          *
          * @param _i2c an instance of a codal::I2C device used to communicate with the onboard accelerometer.
          *
          * @param address the default I2C address of the accelerometer. Defaults to: MMA8453_DEFAULT_ADDR.
          *
          * @param id the unique EventModel id of this component. Defaults to: DEVICE_ID_ACCELEROMETER
          *
          * @code
          * MMA8453 accelerometer = MMA8453(i2c);
          * @endcode
        */
        MMA8453(I2C &_i2c, Pin& int1, CoordinateSpace& cspace, uint16_t address = MMA8453_DEFAULT_ADDR, uint16_t id = DEVICE_ID_ACCELEROMETER) :
          MMA8653(_i2c, int1, cspace, address, id) {}
    };
}

#endif
