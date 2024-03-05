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

#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include "CodalConfig.h"
#include "ManagedString.h"
#include "CodalComponent.h"
#include "Display.h"
#include "Image.h"
#include "Pin.h"
#include "Timer.h"

//
// Internal constants
//
#define LED_MATRIX_GREYSCALE_BIT_DEPTH            8

//
// Event codes raised by an LEDMatrix
//
#define LED_MATRIX_EVT_LIGHT_SENSE                2
#define LED_MATRIX_EVT_FRAME_TIMEOUT              3

//
// Compile Time Configuration Options
//

// Selects the minimum permissable brightness level for the device
// in the region of 0 (off) to 255 (full brightness)
#ifndef LED_MATRIX_MINIMUM_BRIGHTNESS
#define LED_MATRIX_MINIMUM_BRIGHTNESS               1
#endif

// Selects the maximum permissable brightness level for the device
// in the region of 0 (off) to 255 (full brightness)
#ifndef LED_MATRIX_MAXIMUM_BRIGHTNESS
#define LED_MATRIX_MAXIMUM_BRIGHTNESS               255
#endif

// Selects the default brightness for the display
// in the region of zero (off) to 255 (full brightness)
#ifndef LED_MATRIX_DEFAULT_BRIGHTNESS
#define LED_MATRIX_DEFAULT_BRIGHTNESS               LED_MATRIX_MAXIMUM_BRIGHTNESS
#endif

namespace codal
{
    //
    // The different modes that this driver can operate in
    //
    enum DisplayMode {
        DISPLAY_MODE_BLACK_AND_WHITE,
        DISPLAY_MODE_GREYSCALE,
        DISPLAY_MODE_BLACK_AND_WHITE_LIGHT_SENSE,
        DISPLAY_MODE_GREYSCALE_LIGHT_SENSE
    };

    //
    // Valid rotation settings.
    //
    enum DisplayRotation {
        MATRIX_DISPLAY_ROTATION_0,
        MATRIX_DISPLAY_ROTATION_90,
        MATRIX_DISPLAY_ROTATION_180,
        MATRIX_DISPLAY_ROTATION_270
    };

    /**
     * Provides the mapping from Matrix ROW/COL to a linear X/Y buffer.
     * Arranged such that matrixMap[col, row] provides the [x,y] screen co-ordinate.
     */

    struct MatrixPoint
    {
        uint8_t x;
        uint8_t y;
    };
#define NO_CONN 0

    /**
     * This struct presumes rows and columns are arranged contiguously...
     */
    struct MatrixMap
    {
        int         width;                      // The physical width of the LED matrix, in pixels.
        int         height;                     // The physical height of the LED matrix, in pixels.
        int         rows;                       // The number of drive pins connected to LEDs.
        int         columns;                    // The number of sink pins connected to the LEDs.

        Pin         **rowPins;                  // Array of pointers containing an ordered list of pins to drive.
        Pin         **columnPins;               // Array of pointers containing an ordered list of pins to sink.

        const       MatrixPoint *map;           // Table mapping logical LED positions to physical positions.
    };

    /**
     * Class definition for LEDMatrix.
     *
     * Represents an LED matrix array.
     */
    class LEDMatrix : public Display
    {
        uint8_t strobeRow;
        uint8_t rotation;
        uint8_t mode;
        uint8_t greyscaleBitMsk;
        uint8_t timingCount;
        int frameTimeout;

        //
        // State used by all animation routines.
        //

        const MatrixMap &matrixMap;

        // Internal methods to handle animation.

        /**
         *  Called by the display in an interval determined by the brightness of the display, to give an impression
         *  of brightness.
         */
        void renderFinish();

        /**
         * Event handler, called when a requested time has elapsed (used for brightness control).
         */
        void onTimeoutEvent(Event);

        /**
         * Translates a bit mask to a bit mask suitable for the nrf PORT0 and PORT1.
         * Brightness has two levels on, or off.
         */
        void render();

        /**
         * Renders the current image, and drops the fourth frame to allow for
         * sensors that require the display to operate.
         */
        void renderWithLightSense();

        /**
         * Translates a bit mask into a timer interrupt that gives the appearence of greyscale.
         */
        void renderGreyscale();

        /**
         * Enables or disables the display entirely, and releases the pins for other uses.
         *
         * @param enableDisplay true to enabled the display, or false to disable it.
         */
        void setEnable(bool enableDisplay);

        public:

        /**
         * Constructor.
         *
         * Create a software representation of a LED matrix.
         * The display is initially blank.
         *
         * @param map The mapping information that relates pin inputs/outputs to physical screen coordinates.
         * @param id The id the display should use when sending events on the MessageBus. Defaults to DEVICE_ID_DISPLAY.
         *
         */
        LEDMatrix(const MatrixMap &map, uint16_t id = DEVICE_ID_DISPLAY);

        /**
         * Frame update method, invoked periodically to strobe the display.
         */
        virtual void periodicCallback();

        /**
         * Configures the mode of the display.
         *
         * @param mode The mode to swap the display into. One of: DISPLAY_MODE_GREYSCALE,
         *             DISPLAY_MODE_BLACK_AND_WHITE, DISPLAY_MODE_BLACK_AND_WHITE_LIGHT_SENSE
         *
         * @code
         * display.setDisplayMode(DISPLAY_MODE_GREYSCALE); //per pixel brightness
         * @endcode
         */
        void setDisplayMode(DisplayMode mode);

        /**
         * Retrieves the mode of the display.
         *
         * @return the current mode of the display
         */
        int getDisplayMode();

        /**
         * Rotates the display to the given position.
         *
         * Axis aligned values only.
         *
         * @code
         * display.rotateTo(DISPLAY_ROTATION_180); //rotates 180 degrees from original orientation
         * @endcode
         */
        void rotateTo(DisplayRotation position);

        /**
         * Enables the display, should only be called if the display is disabled.
         *
         * @code
         * display.enable(); //Enables the display mechanics
         * @endcode
         *
         * @note Only enables the display if the display is currently disabled.
         */
        virtual void enable();

        /**
         * Disables the display, which releases control of the GPIO pins used by the display,
         * which are exposed on the edge connector.
         *
         * @code
         * display.disable(); //disables the display
         * @endcode
         *
         * @note Only disables the display if the display is currently enabled.
         */
        virtual void disable();

        /**
         * Clears the display of any remaining pixels.
         *
         * `display.image.clear()` can also be used!
         *
         * @code
         * display.clear(); //clears the display
         * @endcode
         */
        void clear();

        /**
         * Configures the brightness of the display.
         *
         * @param b The brightness to set the brightness to, in the range 0 - 255.
         *
         * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER
         */
        virtual int setBrightness(int b);


        /**
         * Destructor for CodalDisplay, where we deregister this instance from the array of system components.
         */
        ~LEDMatrix();
    };
}

#endif