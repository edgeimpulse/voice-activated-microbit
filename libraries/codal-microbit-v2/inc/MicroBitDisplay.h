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

#ifndef MICROBIT_DISPLAY_H
#define MICROBIT_DISPLAY_H

#include "CodalConfig.h"
#include "codal-core/inc/types/Event.h"
#include "NRF52LedMatrix.h"
#include "AnimatedDisplay.h"

namespace codal
{
    /**
     * Class definition for MicroBitDisplay
     */
    class MicroBitDisplay : public NRF52LEDMatrix, public AnimatedDisplay
    {
        public:

        /**
         * Constructor.
         *
         * Create a software representation of the micro:bit's 5x5 LED matrix.
         * The display is initially blank.
         *
         * @param map The mapping information that relates pin inputs/outputs to physical screen coordinates.
         * @param id The id the display should use when sending events on the MessageBus. Defaults to DEVICE_ID_DISPLAY.
         *
         */
        MicroBitDisplay(const MatrixMap &map, uint16_t id = DEVICE_ID_DISPLAY);

        /**
         * Destructor.
         */
        ~MicroBitDisplay();
    };
}

#endif
