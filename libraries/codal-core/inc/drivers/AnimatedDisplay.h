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

#ifndef CODAL_ANIMATED_DISPLAY_H
#define CODAL_ANIMATED_DISPLAY_H

#include "Display.h"
#include "BitmapFont.h"

/**
  * Event codes raised by a Display
  */
#define DISPLAY_EVT_ANIMATION_COMPLETE          1

//
// Internal constants
//
#define DISPLAY_DEFAULT_AUTOCLEAR               1
#define DISPLAY_SPACING                         1
#define DISPLAY_ANIMATE_DEFAULT_POS             -255

//
// Compile time configuration options
//
// Defines the default scroll speed for the display, in the time taken to move a single pixel (ms).
#ifndef DISPLAY_DEFAULT_SCROLL_SPEED
#define DISPLAY_DEFAULT_SCROLL_SPEED                120
#endif

// Selects the number of pixels a scroll will move in each quantum.
#ifndef DISPLAY_DEFAULT_SCROLL_STRIDE
#define DISPLAY_DEFAULT_SCROLL_STRIDE       -1
#endif

// Selects the time each character will be shown on the display during print operations.
// The time each character is shown on the screen  (ms).
#ifndef DISPLAY_DEFAULT_PRINT_SPEED
#define DISPLAY_DEFAULT_PRINT_SPEED         400
#endif

namespace codal
{
    enum AnimationMode {
        ANIMATION_MODE_NONE,
        ANIMATION_MODE_STOPPED,
        ANIMATION_MODE_SCROLL_TEXT,
        ANIMATION_MODE_PRINT_TEXT,
        ANIMATION_MODE_SCROLL_IMAGE,
        ANIMATION_MODE_ANIMATE_IMAGE,
        ANIMATION_MODE_ANIMATE_IMAGE_WITH_CLEAR,
        ANIMATION_MODE_PRINT_CHARACTER
    };

    /**
     * Class definition for AnimatedDisplay.
     *
     * This class provides a high level abstraction level to show text and graphic animations on a
     * Display, e.g. on the LED matrix display.
     */
    class AnimatedDisplay : public CodalComponent
    {
        // The Display instance that is used to show the text and graphic animations of this class
        Display& display;

        // The Font to use text operations (TODO: This is currently only partially implemented)
        BitmapFont font;

        //
        // State used by all animation routines.
        //

        // The animation mode that's currently running (if any)
        volatile AnimationMode animationMode;

        // The time in milliseconds between each frame update.
        uint16_t animationDelay;

        // The time in milliseconds since the frame update.
        uint16_t animationTick;

        // Stop playback of any animations
        void stopAnimation(int delay);

        //
        // State for scrollString() method.
        // This is a surprisingly intricate method.
        //
        // The text being displayed.
        ManagedString scrollingText;

        // The index of the character currently being displayed.
        int16_t scrollingChar;

        // The number of pixels the current character has been shifted on the display.
        int8_t scrollingPosition;

        //
        // State for printString() method.
        //
        // The text being displayed. NULL if no message is scheduled for playback.
        // We *could* get some reuse in here with the scroll* variables above,
        // but best to keep it clean in case kids try concurrent operation (they will!),
        // given the small RAM overhead needed to maintain orthogonality.
        ManagedString printingText;

        // The index of the character currently being displayed.
        int16_t printingChar;

        //
        // State for scrollImage() method.
        //
        // The image being displayed.
        Image scrollingImage;

        // The number of pixels the image has been shifted on the display.
        int16_t scrollingImagePosition;

        // The number of pixels the image is shifted on the display in each quantum.
        int8_t scrollingImageStride;

        // Flag to indicate if image has been rendered to screen yet (or not)
        bool scrollingImageRendered;

        private:
        // Internal methods to handle animation.

        /**
         *  Periodic callback, that we use to perform any animations we have running.
         */
        void animationUpdate();


        /**
         * Internal scrollText update method.
         * Shift the screen image by one pixel to the left. If necessary, paste in the next char.
         */
        void updateScrollText();

        /**
         * Internal printText update method.
         * Paste the next character in the string.
         */
        void updatePrintText();

        /**
         * Internal scrollImage update method.
         * Paste the stored bitmap at the appropriate point.
         */
        void updateScrollImage();

        /**
         * Internal animateImage update method.
         * Paste the stored bitmap at the appropriate point and stop on the last frame.
         */
        void updateAnimateImage();

        /**
         * Broadcasts an event onto the default EventModel indicating that the
         * current animation has completed.
         */
        void sendAnimationCompleteEvent();

        /**
         * Blocks the current fiber until the display is available (i.e. does not effect is being displayed).
         * Animations are queued until their time to display.
         */
        void waitForFreeDisplay();

        /**
         * Blocks the current fiber until the current animation has finished.
         * If the scheduler is not running, this call will essentially perform a spinning wait.
         */
        void fiberWait();

        public:
        /**
         * Constructor.
         *
         * Create a software representation of an animated display.
         * This class provides a high level abstraction level to show text and graphic animations on a
         * CodalDisplay, e.g. on the LED matrix display.
         *
         * @param display The CodalDisplay instance that is used to show the text and graphic animations of this class
         *
         * @param id The id the display should use when sending events on the MessageBus. Defaults to DEVICE_ID_DISPLAY.
         *
         */
        AnimatedDisplay(Display& display, uint16_t id = DEVICE_ID_DISPLAY);

        /**
         * Frame update method, invoked periodically to strobe the display.
         */
        virtual void periodicCallback();

        /**
         * Stops any currently running animation, and any that are waiting to be displayed.
         */
        void stopAnimation();

        /**
         * Prints the given character to the display, if it is not in use.
         *
         * @param c The character to display.
         *
         * @param delay Optional parameter - the time for which to show the character. Zero displays the character forever,
         *              or until the Displays next use.
         *
         * @return DEVICE_OK, DEVICE_BUSY is the screen is in use, or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * display.printCharAsync('p');
         * display.printCharAsync('p',100);
         * @endcode
         */
        int printCharAsync(char c, int delay = 0);

        /**
         * Prints the given ManagedString to the display, one character at a time.
         * Returns immediately, and executes the animation asynchronously.
         *
         * @param s The string to display.
         *
         * @param delay The time to delay between characters, in milliseconds. Must be > 0.
         *              Defaults to: DISPLAY_DEFAULT_PRINT_SPEED.
         *
         * @return DEVICE_OK, or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * display.printAsync("abc123",400);
         * @endcode
         */
        int printAsync(ManagedString s, int delay = DISPLAY_DEFAULT_PRINT_SPEED);

        /**
         * Prints the given image to the display, if the display is not in use.
         * Returns immediately, and executes the animation asynchronously.
         *
         * @param i The image to display.
         *
         * @param x The horizontal position on the screen to display the image. Defaults to 0.
         *
         * @param y The vertical position on the screen to display the image. Defaults to 0.
         *
         * @param alpha Treats the brightness level '0' as transparent. Defaults to 0.
         *
         * @param delay The time to delay between characters, in milliseconds. Defaults to 0.
         *
         * @code
         * Image i("1,1,1,1,1\n1,1,1,1,1\n");
         * display.printAsync(i,400);
         * @endcode
         */
        int printAsync(Image i, int x = 0, int y = 0, int alpha = 0, int delay = 0);

        /**
         * Prints the given character to the display.
         *
         * @param c The character to display.
         *
         * @param delay Optional parameter - the time for which to show the character. Zero displays the character forever,
         *              or until the Displays next use.
         *
         * @return DEVICE_OK, DEVICE_CANCELLED or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * display.printAsync('p');
         * display.printAsync('p',100);
         * @endcode
         */
        int printChar(char c, int delay = 0);

        /**
         * Prints the given string to the display, one character at a time.
         *
         * Blocks the calling thread until all the text has been displayed.
         *
         * @param s The string to display.
         *
         * @param delay The time to delay between characters, in milliseconds. Defaults
         *              to: DISPLAY_DEFAULT_PRINT_SPEED.
         *
         * @return DEVICE_OK, DEVICE_CANCELLED or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * display.print("abc123",400);
         * @endcode
         */
        int print(ManagedString s, int delay = DISPLAY_DEFAULT_PRINT_SPEED);

        /**
         * Prints the given image to the display.
         * Blocks the calling thread until all the image has been displayed.
         *
         * @param i The image to display.
         *
         * @param x The horizontal position on the screen to display the image. Defaults to 0.
         *
         * @param y The vertical position on the screen to display the image. Defaults to 0.
         *
         * @param alpha Treats the brightness level '0' as transparent. Defaults to 0.
         *
         * @param delay The time to display the image for, or zero to show the image forever. Defaults to 0.
         *
         * @return DEVICE_OK, DEVICE_BUSY if the display is already in use, or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * Image i("1,1,1,1,1\n1,1,1,1,1\n");
         * display.print(i,400);
         * @endcode
         */
        int print(Image i, int x = 0, int y = 0, int alpha = 0, int delay = 0);

        /**
         * Scrolls the given string to the display, from right to left.
         * Returns immediately, and executes the animation asynchronously.
         *
         * @param s The string to display.
         *
         * @param delay The time to delay between characters, in milliseconds. Defaults
         *              to: DISPLAY_DEFAULT_SCROLL_SPEED.
         *
         * @return DEVICE_OK, DEVICE_BUSY if the display is already in use, or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * display.scrollAsync("abc123",100);
         * @endcode
         */
        int scrollAsync(ManagedString s, int delay = DISPLAY_DEFAULT_SCROLL_SPEED);

        /**
         * Scrolls the given image across the display, from right to left.
         * Returns immediately, and executes the animation asynchronously.
         *
         * @param image The image to display.
         *
         * @param delay The time between updates, in milliseconds. Defaults
         *              to: DISPLAY_DEFAULT_SCROLL_SPEED.
         *
         * @param stride The number of pixels to shift by in each update. Defaults to DISPLAY_DEFAULT_SCROLL_STRIDE.
         *
         * @return DEVICE_OK, DEVICE_BUSY if the display is already in use, or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * Image i("1,1,1,1,1\n1,1,1,1,1\n");
         * display.scrollAsync(i,100,1);
         * @endcode
         */
        int scrollAsync(Image image, int delay = DISPLAY_DEFAULT_SCROLL_SPEED, int stride = DISPLAY_DEFAULT_SCROLL_STRIDE);

        /**
         * Scrolls the given string across the display, from right to left.
         * Blocks the calling thread until all text has been displayed.
         *
         * @param s The string to display.
         *
         * @param delay The time to delay between characters, in milliseconds. Defaults
         *              to: DISPLAY_DEFAULT_SCROLL_SPEED.
         *
         * @return DEVICE_OK, DEVICE_CANCELLED or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * display.scroll("abc123",100);
         * @endcode
         */
        int scroll(ManagedString s, int delay = DISPLAY_DEFAULT_SCROLL_SPEED);

        /**
         * Scrolls the given image across the display, from right to left.
         * Blocks the calling thread until all the text has been displayed.
         *
         * @param image The image to display.
         *
         * @param delay The time between updates, in milliseconds. Defaults
         *              to: DISPLAY_DEFAULT_SCROLL_SPEED.
         *
         * @param stride The number of pixels to shift by in each update. Defaults to DISPLAY_DEFAULT_SCROLL_STRIDE.
         *
         * @return DEVICE_OK, DEVICE_CANCELLED or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * Image i("1,1,1,1,1\n1,1,1,1,1\n");
         * display.scroll(i,100,1);
         * @endcode
         */
        int scroll(Image image, int delay = DISPLAY_DEFAULT_SCROLL_SPEED, int stride = DISPLAY_DEFAULT_SCROLL_STRIDE);

        /**
         * "Animates" the current image across the display with a given stride, finishing on the last frame of the animation.
         * Returns immediately.
         *
         * @param image The image to display.
         *
         * @param delay The time to delay between each update of the display, in milliseconds.
         *
         * @param stride The number of pixels to shift by in each update.
         *
         * @param startingPosition the starting position on the display for the animation
         *                         to begin at. Defaults to DISPLAY_ANIMATE_DEFAULT_POS.
         *
         * @param autoClear defines whether or not the display is automatically cleared once the animation is complete. By default, the display is cleared. Set this parameter to zero to disable the autoClear operation.
         *
         * @return DEVICE_OK, DEVICE_BUSY if the screen is in use, or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * const int heart_w = 10;
         * const int heart_h = 5;
         * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, };
         *
         * Image i(heart_w,heart_h,heart);
         * display.animateAsync(i,100,5);
         * @endcode
         */
        int animateAsync(Image image, int delay, int stride, int startingPosition = DISPLAY_ANIMATE_DEFAULT_POS, int autoClear = DISPLAY_DEFAULT_AUTOCLEAR);

        /**
         * "Animates" the current image across the display with a given stride, finishing on the last frame of the animation.
         * Blocks the calling thread until the animation is complete.
         *
         *
         * @param delay The time to delay between each update of the display, in milliseconds.
         *
         * @param stride The number of pixels to shift by in each update.
         *
         * @param startingPosition the starting position on the display for the animation
         *                         to begin at. Defaults to DISPLAY_ANIMATE_DEFAULT_POS.
         *
         * @param autoClear defines whether or not the display is automatically cleared once the animation is complete. By default, the display is cleared. Set this parameter to zero to disable the autoClear operation.
         *
         * @return DEVICE_OK, DEVICE_CANCELLED or DEVICE_INVALID_PARAMETER.
         *
         * @code
         * const int heart_w = 10;
         * const int heart_h = 5;
         * const uint8_t heart[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, };
         *
         * Image i(heart_w,heart_h,heart);
         * display.animate(i,100,5);
         * @endcode
         */
        int animate(Image image, int delay, int stride, int startingPosition = DISPLAY_ANIMATE_DEFAULT_POS, int autoClear = DISPLAY_DEFAULT_AUTOCLEAR);

        /**
         * Destructor for AnimatedDisplay, where we deregister this instance from the array of system components.
         */
        ~AnimatedDisplay();

    };
}

#endif
