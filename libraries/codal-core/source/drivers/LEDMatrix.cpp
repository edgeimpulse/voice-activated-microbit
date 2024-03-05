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

/**
  * Class definition for LEDMatrix.
  *
  * Represents an LED matrix array.
  */
#include "LEDMatrix.h"
#include "CodalFiber.h"
#include "CodalDmesg.h"
#include "ErrorNo.h"

using namespace codal;

const int greyScaleTimings[LED_MATRIX_GREYSCALE_BIT_DEPTH] = {1, 23, 70, 163, 351, 726, 1476, 2976};

/**
  * Constructor.
  *
  * Create a software representation an LED matrix.
  * The display is initially blank.
  *
  * @param map The mapping information that relates pin inputs/outputs to physical screen coordinates.
  * @param id The id the display should use when sending events on the MessageBus. Defaults to DEVICE_ID_DISPLAY.
  */
LEDMatrix::LEDMatrix(const MatrixMap &map, uint16_t id) : Display(map.width, map.height, id), matrixMap(map)
{
    this->rotation = MATRIX_DISPLAY_ROTATION_0;
    this->greyscaleBitMsk = 0x01;
    this->timingCount = 0;
    this->setBrightness(LED_MATRIX_DEFAULT_BRIGHTNESS);
    this->mode = DISPLAY_MODE_BLACK_AND_WHITE;
    this->strobeRow = 0;

    if(EventModel::defaultEventBus)
        EventModel::defaultEventBus->listen(id, LED_MATRIX_EVT_FRAME_TIMEOUT, this, &LEDMatrix::onTimeoutEvent, MESSAGE_BUS_LISTENER_IMMEDIATE);

    this->status |= DEVICE_COMPONENT_STATUS_SYSTEM_TICK;
    this->status |= DEVICE_COMPONENT_RUNNING;
}

/**
  * Internal frame update method, used to strobe the display.
  *
  * TODO: Write a more efficient, complementary variation of this method for the case where
  * we have more rows than columns.
  */
void LEDMatrix::periodicCallback()
{
    if(!(status & DEVICE_COMPONENT_RUNNING))
        return;

    if(mode == DISPLAY_MODE_BLACK_AND_WHITE_LIGHT_SENSE)
    {
        renderWithLightSense();
        return;
    }

    if(mode == DISPLAY_MODE_BLACK_AND_WHITE)
        render();

    if(mode == DISPLAY_MODE_GREYSCALE)
    {
        greyscaleBitMsk = 0x01;
        timingCount = 0;
        renderGreyscale();
    }
}

void LEDMatrix::renderFinish()
{
    matrixMap.rowPins[strobeRow]->setDigitalValue(0);
}

void LEDMatrix::onTimeoutEvent(Event)
{
    renderFinish();
}

void LEDMatrix::render()
{
    // Simple optimisation.
    // If display is at zero brightness, there's nothing to do.
    if(brightness == 0)
        return;

    // Turn off the previous row
    matrixMap.rowPins[strobeRow]->setDigitalValue(0);
    matrixMap.rowPins[strobeRow]->getDigitalValue();

    // Move on to the next row.
    strobeRow++;
    if(strobeRow == matrixMap.rows)
        strobeRow = 0;

    // Calculate the bitpattern to write.
    for (int i = 0; i < matrixMap.columns; i++)
    {
        int index = (i * matrixMap.rows) + strobeRow;

        int x = matrixMap.map[index].x;
        int y = matrixMap.map[index].y;
        int t = x;

        if(rotation == MATRIX_DISPLAY_ROTATION_90)
        {
                x = width - 1 - y;
                y = t;
        }

        if(rotation == MATRIX_DISPLAY_ROTATION_180)
        {
                x = width - 1 - x;
                y = height - 1 - y;
        }

        if(rotation == MATRIX_DISPLAY_ROTATION_270)
        {
                x = y;
                y = height - 1 - t;
        }
        if (image.getBitmap()[y*width + x])
            matrixMap.columnPins[i]->setDigitalValue(0);
        else
            matrixMap.columnPins[i]->setDigitalValue(1);
    }

    // Turn off the previous row
    matrixMap.rowPins[strobeRow]->setDigitalValue(1);

    //timer does not have enough resolution for brightness of 1. 23.53 us
    if(brightness <= LED_MATRIX_MAXIMUM_BRIGHTNESS && brightness > LED_MATRIX_MINIMUM_BRIGHTNESS)
        system_timer_event_after_us(frameTimeout, id, LED_MATRIX_EVT_FRAME_TIMEOUT);

    //this will take around 23us to execute
    if(brightness <= LED_MATRIX_MINIMUM_BRIGHTNESS)
        renderFinish();
}

void LEDMatrix::renderWithLightSense()
{
    //reset the row counts and bit mask when we have hit the max.
    if(strobeRow == matrixMap.rows + 1)
    {
        Event(id, LED_MATRIX_EVT_LIGHT_SENSE);
        strobeRow = 0;
    }
    else
    {
        render();
    }

}

void LEDMatrix::renderGreyscale()
{
/*
    uint32_t row_data = 0x01 << (matrixMap.rowStart + strobeRow);
    uint32_t col_data = 0;

    // Calculate the bitpattern to write.
    for (int i = 0; i < matrixMap.columns; i++)
    {
        int index = (i * matrixMap.rows) + strobeRow;

        int x = matrixMap.map[index].x;
        int y = matrixMap.map[index].y;
        int t = x;

        if(rotation == MATRIX_DISPLAY_ROTATION_90)
        {
                x = width - 1 - y;
                y = t;
        }

        if(rotation == MATRIX_DISPLAY_ROTATION_180)
        {
                x = width - 1 - x;
                y = height - 1 - y;
        }

        if(rotation == MATRIX_DISPLAY_ROTATION_270)
        {
                x = y;
                y = height - 1 - t;
        }

        if(min(image.getBitmap()[y * width + x],brightness) & greyscaleBitMsk)
            col_data |= (1 << i);
    }

    // Invert column bits (as we're sinking not sourcing power), and mask off any unused bits.
    col_data = ~col_data << matrixMap.columnStart & col_mask;

    // Write the new bit pattern
    *LEDMatrix = col_data | row_data;

    if(timingCount > CODAL_DISPLAY_GREYSCALE_BIT_DEPTH-1)
        return;

    greyscaleBitMsk <<= 1;

    if(timingCount < 3)
    {
        wait_us(greyScaleTimings[timingCount++]);
        renderGreyscale();
        return;
    }
    renderTimer.attach_us(this,&LEDMatrix::renderGreyscale, greyScaleTimings[timingCount++]);
*/
}

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
void LEDMatrix::setDisplayMode(DisplayMode mode)
{
    this->mode = mode;
}

/**
  * Retrieves the mode of the display.
  *
  * @return the current mode of the display
  */
int LEDMatrix::getDisplayMode()
{
    return this->mode;
}

/**
  * Rotates the display to the given position.
  *
  * Axis aligned values only.
  *
  * @code
  * display.rotateTo(MATRIX_DISPLAY_ROTATION_180); //rotates 180 degrees from original orientation
  * @endcode
  */
void LEDMatrix::rotateTo(DisplayRotation rotation)
{
    this->rotation = rotation;
}

/**
 * Enables or disables the display entirely, and releases the pins for other uses.
 *
 * @param enableDisplay true to enabled the display, or false to disable it.
 */
void LEDMatrix::setEnable(bool enableDisplay)
{
    // If we're already in the correct state, then there's nothing to do.
    if(((status & DEVICE_COMPONENT_RUNNING) && enableDisplay) || (!(status & DEVICE_COMPONENT_RUNNING) && !enableDisplay))
        return;

    // Turn off the currently live row row
    matrixMap.rowPins[strobeRow]->getDigitalValue();

    if (enableDisplay)
    {
        status |= DEVICE_COMPONENT_RUNNING;
    }
    else
    {
        status &= ~DEVICE_COMPONENT_RUNNING;
    }
}

/**
  * Enables the display, should only be called if the display is disabled.
  *
  * @code
  * display.enable(); //Enables the display mechanics
  * @endcode
  *
  * @note Only enables the display if the display is currently disabled.
  */
void LEDMatrix::enable()
{
    setEnable(true);
}

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
void LEDMatrix::disable()
{
    setEnable(false);
}

/**
  * Clears the display of any remaining pixels.
  *
  * `display.image.clear()` can also be used!
  *
  * @code
  * display.clear(); //clears the display
  * @endcode
  */
void LEDMatrix::clear()
{
    image.clear();
}

/**
 * Configures the brightness of the display.
 *
 * @param b The brightness to set the brightness to, in the range 0 - 255.
 *
 * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER
 */
int LEDMatrix::setBrightness(int b)
{
    int result = Display::setBrightness(b);

    if (result != DEVICE_OK)
        return result;

    // Precalculate the per frame "on" time for this brightness level.
    frameTimeout = (((int)brightness) * 1024 * SCHEDULER_TICK_PERIOD_US) / (255 * 1024);

    return DEVICE_OK;
}

/**
  * Destructor for LEDMatrix, where we deregister this instance from the array of system components.
  */
LEDMatrix::~LEDMatrix()
{
    this->status &= ~DEVICE_COMPONENT_STATUS_SYSTEM_TICK;
}