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
  * Class definition for AnimatedDisplay.
  *
  * This class provides a high level abstraction level to show text and graphic animations on a
  * Display, e.g. on an LED matrix display.
  */

#include "AnimatedDisplay.h"
#include "CodalFiber.h"
#include "NotifyEvents.h"
#include "CodalDmesg.h"

using namespace codal;

 /**
  * Constructor.
  *
  * Create a software representation of an animated display.
  * This class provides a high level abstraction level to show text and graphic animations on a
  * Display, e.g. on an LED matrix display.
  *
  * @param display The Display instance that is used to show the text and graphic animations of this class
  * @param id The id the display should use when sending events on the MessageBus. Defaults to DEVICE_ID_DISPLAY.
  *
  */
AnimatedDisplay::AnimatedDisplay(Display& _display, uint16_t id) : display(_display), font(), printingText(), scrollingImage()
{
    this->id = id;
    this->status = 0;

    status |= DEVICE_COMPONENT_STATUS_SYSTEM_TICK;
    status |= DEVICE_COMPONENT_RUNNING;

    animationMode = AnimationMode::ANIMATION_MODE_NONE;
    animationDelay = 0;
    animationTick = 0;
    scrollingChar = 0;
    scrollingPosition = 0;
    printingChar = 0;
    scrollingImagePosition = 0;
    scrollingImageStride = 0;
    scrollingImageRendered = false;
}

/**
  * Periodic callback, that we use to perform any animations we have running.
  */
void AnimatedDisplay::animationUpdate()
{
    // If there's no ongoing animation, then nothing to do.
    if (animationMode == ANIMATION_MODE_NONE)
        return;

    animationTick += (SCHEDULER_TICK_PERIOD_US/1000);

    if(animationTick >= animationDelay)
    {
        animationTick = 0;

        if (animationMode == ANIMATION_MODE_SCROLL_TEXT)
            this->updateScrollText();

        if (animationMode == ANIMATION_MODE_PRINT_TEXT)
            this->updatePrintText();

        if (animationMode == ANIMATION_MODE_SCROLL_IMAGE)
            this->updateScrollImage();

        if (animationMode == ANIMATION_MODE_ANIMATE_IMAGE || animationMode == ANIMATION_MODE_ANIMATE_IMAGE_WITH_CLEAR)
            this->updateAnimateImage();

        if(animationMode == ANIMATION_MODE_PRINT_CHARACTER)
        {
            animationMode = ANIMATION_MODE_NONE;
            this->sendAnimationCompleteEvent();
        }
    }
}

/**
  * Broadcasts an event onto the defult EventModel indicating that the
  * current animation has completed.
  */
void AnimatedDisplay::sendAnimationCompleteEvent()
{
    // Signal that we've completed an animation.
    Event(id, DISPLAY_EVT_ANIMATION_COMPLETE);

    // Wake up a fiber that was blocked on the animation (if any).
    Event(DEVICE_ID_NOTIFY_ONE, DISPLAY_EVT_FREE);
}

/**
  * Internal scrollText update method.
  * Shift the screen image by one pixel to the left. If necessary, paste in the next char.
  */
void AnimatedDisplay::updateScrollText()
{
    /**
    display.image.shiftLeft(1);
    scrollingPosition++;

    if (scrollingPosition == display.getWidth() + DISPLAY_SPACING)
    {
        scrollingPosition = 0;

        display.image.print(scrollingChar < scrollingText.length() ? scrollingText.charAt(scrollingChar) : ' ', display.getWidth(), 0);

        if (scrollingChar > scrollingText.length())
        {
            animationMode = ANIMATION_MODE_NONE;
            this->sendAnimationCompleteEvent();
            return;
        }
        scrollingChar++;
   }
   **/

    display.image.shiftLeft(1);

    if (scrollingPosition < BITMAP_FONT_WIDTH && scrollingChar < scrollingText.length())
    {
        const uint8_t *v = font.get(scrollingText.charAt(scrollingChar));
        uint8_t mask = 1 << (BITMAP_FONT_WIDTH - scrollingPosition - 1);
        uint8_t x = display.getWidth()-1;

        for (int y=0; y<BITMAP_FONT_HEIGHT; y++)
        {
            if (*v & mask)
                display.image.setPixelValue(x, y, 255);

            v++;
        }
    }

    scrollingPosition++;

    if (scrollingPosition == display.getWidth() + DISPLAY_SPACING)
    {
        scrollingPosition = 0;

        if (scrollingChar >= scrollingText.length())
        {
            animationMode = ANIMATION_MODE_NONE;
            this->sendAnimationCompleteEvent();
            return;
        }
        scrollingChar++;
    }
}

/**
  * Internal printText update method.
  * Paste the next character in the string.
  */
void AnimatedDisplay::updatePrintText()
{
    display.image.print(printingChar < printingText.length() ? printingText.charAt(printingChar) : ' ',0,0);

    if (printingChar > printingText.length())
    {
        animationMode = ANIMATION_MODE_NONE;

        this->sendAnimationCompleteEvent();
        return;
    }

    printingChar++;
}

/**
  * Internal scrollImage update method.
  * Paste the stored bitmap at the appropriate point.
  */
void AnimatedDisplay::updateScrollImage()
{
    display.image.clear();

    if (((display.image.paste(scrollingImage, scrollingImagePosition, 0, 0) == 0) && scrollingImageRendered) || scrollingImageStride == 0)
    {
        animationMode = ANIMATION_MODE_NONE;
        this->sendAnimationCompleteEvent();

        return;
    }

    scrollingImagePosition += scrollingImageStride;
    scrollingImageRendered = true;
}

/**
  * Internal animateImage update method.
  * Paste the stored bitmap at the appropriate point and stop on the last frame.
  */
void AnimatedDisplay::updateAnimateImage()
{
    //wait until we have rendered the last position to give a continuous animation.
    if (scrollingImagePosition <= -scrollingImage.getWidth() + (display.getWidth() + scrollingImageStride) && scrollingImageRendered)
    {
        if (animationMode == ANIMATION_MODE_ANIMATE_IMAGE_WITH_CLEAR)
            display.image.clear();

        animationMode = ANIMATION_MODE_NONE;

        this->sendAnimationCompleteEvent();
        return;
    }

    if(scrollingImagePosition > 0)
        display.image.shiftLeft(-scrollingImageStride);

    display.image.paste(scrollingImage, scrollingImagePosition, 0, 0);

    if(scrollingImageStride == 0)
    {
        animationMode = ANIMATION_MODE_NONE;
        this->sendAnimationCompleteEvent();
    }

    scrollingImageRendered = true;

    scrollingImagePosition += scrollingImageStride;
}

/**
  * Resets the current given animation.
  */
void AnimatedDisplay::stopAnimation()
{
    // Reset any ongoing animation.
    if (animationMode != ANIMATION_MODE_NONE)
    {
        animationMode = ANIMATION_MODE_NONE;

        // Indicate that we've completed an animation.
        Event(id,DISPLAY_EVT_ANIMATION_COMPLETE);

        // Wake up aall fibers that may blocked on the animation (if any).
        Event(DEVICE_ID_NOTIFY, DISPLAY_EVT_FREE);
    }

    // Clear the display and setup the animation timers.
    this->display.image.clear();
}

/**
  * Blocks the current fiber until the display is available (i.e. does not effect is being displayed).
  * Animations are queued until their time to display.
  */
void AnimatedDisplay::waitForFreeDisplay()
{
    // If there's an ongoing animation, wait for our turn to display.
    if (animationMode != ANIMATION_MODE_NONE && animationMode != ANIMATION_MODE_STOPPED)
        fiber_wait_for_event(DEVICE_ID_NOTIFY, DISPLAY_EVT_FREE);
}

/**
  * Blocks the current fiber until the current animation has finished.
  * If the scheduler is not running, this call will essentially perform a spinning wait.
  */
void AnimatedDisplay::fiberWait()
{
    if (fiber_wait_for_event(DEVICE_ID_DISPLAY, DISPLAY_EVT_ANIMATION_COMPLETE) == DEVICE_NOT_SUPPORTED)
        while(animationMode != ANIMATION_MODE_NONE && animationMode != ANIMATION_MODE_STOPPED)
            target_wait_for_event();
}

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
int AnimatedDisplay::printCharAsync(char c, int delay)
{
    //sanitise this value
    if(delay < 0)
        return DEVICE_INVALID_PARAMETER;

    // If the display is free, it's our turn to display.
    if (animationMode == ANIMATION_MODE_NONE || animationMode == ANIMATION_MODE_STOPPED)
    {
        display.image.print(c, 0, 0);

        if (delay > 0)
        {
            animationDelay = delay;
            animationTick = 0;
            animationMode = ANIMATION_MODE_PRINT_CHARACTER;
        }
    }
    else
    {
        return DEVICE_BUSY;
    }

    return DEVICE_OK;
}

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
int AnimatedDisplay::printAsync(ManagedString s, int delay)
{
    if (s.length() == 1)
        return printCharAsync(s.charAt(0));

    //sanitise this value
    if (delay <= 0 )
        return DEVICE_INVALID_PARAMETER;

    if (animationMode == ANIMATION_MODE_NONE || animationMode == ANIMATION_MODE_STOPPED)
    {
        printingChar = 0;
        printingText = s;
        animationDelay = delay;
        animationTick = 0;

        animationMode = ANIMATION_MODE_PRINT_TEXT;
    }
    else
    {
        return DEVICE_BUSY;
    }

    return DEVICE_OK;
}

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
  * display.printprintAsync(i,400);
  * @endcode
  */
int AnimatedDisplay::printAsync(Image i, int x, int y, int alpha, int delay)
{
    if(delay < 0)
        return DEVICE_INVALID_PARAMETER;

    if (animationMode == ANIMATION_MODE_NONE || animationMode == ANIMATION_MODE_STOPPED)
    {
        display.image.paste(i, x, y, alpha);

        if(delay > 0)
        {
            animationDelay = delay;
            animationTick = 0;
            animationMode = ANIMATION_MODE_PRINT_CHARACTER;
        }
    }
    else
    {
        return DEVICE_BUSY;
    }

    return DEVICE_OK;
}

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
int AnimatedDisplay::printChar(char c, int delay)
{
    if (delay < 0)
        return DEVICE_INVALID_PARAMETER;

    // If there's an ongoing animation, wait for our turn to display.
    this->waitForFreeDisplay();

    // If the display is free, it's our turn to display.
    // If someone called stopAnimation(), then we simply skip...
    if (animationMode == ANIMATION_MODE_NONE)
    {
        this->printCharAsync(c, delay);

        if (delay > 0)
            fiberWait();
    }
    else
    {
        return DEVICE_CANCELLED;
    }

    return DEVICE_OK;
}

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
int AnimatedDisplay::print(ManagedString s, int delay)
{
    //sanitise this value
    if(delay <= 0 )
        return DEVICE_INVALID_PARAMETER;

    // If there's an ongoing animation, wait for our turn to display.
    this->waitForFreeDisplay();

    // If the display is free, it's our turn to display.
    // If someone called stopAnimation(), then we simply skip...
    if (animationMode == ANIMATION_MODE_NONE)
    {
        if (s.length() == 1)
        {
            return printCharAsync(s.charAt(0));
        }
        else
        {
            this->printAsync(s, delay);
            fiberWait();
        }
    }
    else
    {
        return DEVICE_CANCELLED;
    }

    return DEVICE_OK;
}

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
int AnimatedDisplay::print(Image i, int x, int y, int alpha, int delay)
{
    if(delay < 0)
        return DEVICE_INVALID_PARAMETER;

    // If there's an ongoing animation, wait for our turn to display.
    this->waitForFreeDisplay();

    // If the display is free, it's our turn to display.
    // If someone called stopAnimation(), then we simply skip...
    if (animationMode == ANIMATION_MODE_NONE)
    {
        this->printAsync(i, x, y, alpha, delay);

        if (delay > 0)
            fiberWait();
    }
    else
    {
        return DEVICE_CANCELLED;
    }

    return DEVICE_OK;
}

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
int AnimatedDisplay::scrollAsync(ManagedString s, int delay)
{
    //sanitise this value
    if(delay <= 0)
        return DEVICE_INVALID_PARAMETER;

    // If the display is free, it's our turn to display.
    if (animationMode == ANIMATION_MODE_NONE || animationMode == ANIMATION_MODE_STOPPED)
    {
        scrollingPosition = 0;
        scrollingChar = 0;
        scrollingText = s;

        animationDelay = delay;
        animationTick = 0;
        animationMode = ANIMATION_MODE_SCROLL_TEXT;
    }
    else
    {
        return DEVICE_BUSY;
    }

    return DEVICE_OK;
}

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
int AnimatedDisplay::scrollAsync(Image image, int delay, int stride)
{
    //sanitise the delay value
    if(delay <= 0)
        return DEVICE_INVALID_PARAMETER;

    // If the display is free, it's our turn to display.
    if (animationMode == ANIMATION_MODE_NONE || animationMode == ANIMATION_MODE_STOPPED)
    {
        scrollingImagePosition = stride < 0 ? display.getWidth() : -image.getWidth();
        scrollingImageStride = stride;
        scrollingImage = image;
        scrollingImageRendered = false;

        animationDelay = stride == 0 ? 0 : delay;
        animationTick = 0;
        animationMode = ANIMATION_MODE_SCROLL_IMAGE;
    }
    else
    {
        return DEVICE_BUSY;
    }

    return DEVICE_OK;
}

/**
  * Scrolls the given string across the display, from right to left.
  * Blocks the calling thread until all text has been displayed.
  *
  * @param s The string to display.
  *
  * @param delay The time to delay between characters, in milliseconds. Defaults
  *              to: DEVICE_DEFAULT_SCROLL_SPEED.
  *
  * @return DEVICE_OK, DEVICE_CANCELLED or DEVICE_INVALID_PARAMETER.
  *
  * @code
  * display.scroll("abc123",100);
  * @endcode
  */
int AnimatedDisplay::scroll(ManagedString s, int delay)
{
    //sanitise this value
    if(delay <= 0)
        return DEVICE_INVALID_PARAMETER;

    // If there's an ongoing animation, wait for our turn to display.
    this->waitForFreeDisplay();

    // If the display is free, it's our turn to display.
    // If someone called stopAnimation(), then we simply skip...
    if (animationMode == ANIMATION_MODE_NONE)
    {
        // Start the effect.
        this->scrollAsync(s, delay);

        // Wait for completion.
        fiberWait();
    }
    else
    {
        return DEVICE_CANCELLED;
    }

    return DEVICE_OK;
}

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
int AnimatedDisplay::scroll(Image image, int delay, int stride)
{
    //sanitise the delay value
    if(delay <= 0)
        return DEVICE_INVALID_PARAMETER;

    // If there's an ongoing animation, wait for our turn to display.
    this->waitForFreeDisplay();

    // If the display is free, it's our turn to display.
    // If someone called stopAnimation(), then we simply skip...
    if (animationMode == ANIMATION_MODE_NONE)
    {
        // Start the effect.
        this->scrollAsync(image, delay, stride);

        // Wait for completion.
        fiberWait();
    }
    else
    {
        return DEVICE_CANCELLED;
    }

    return DEVICE_OK;
}

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
int AnimatedDisplay::animateAsync(Image image, int delay, int stride, int startingPosition, int autoClear)
{
    //sanitise the delay value
    if(delay <= 0)
        return DEVICE_INVALID_PARAMETER;

    // If the display is free, we can display.
    if (animationMode == ANIMATION_MODE_NONE || animationMode == ANIMATION_MODE_STOPPED)
    {
        // Assume right to left functionality, to align with scrollString()
        stride = -stride;

        //calculate starting position which is offset by the stride
        scrollingImagePosition = (startingPosition == DISPLAY_ANIMATE_DEFAULT_POS) ? display.getWidth() + stride : startingPosition;
        scrollingImageStride = stride;
        scrollingImage = image;
        scrollingImageRendered = false;

        animationDelay = stride == 0 ? 0 : delay;
        animationTick = delay-1;
        animationMode = autoClear ? ANIMATION_MODE_ANIMATE_IMAGE_WITH_CLEAR : ANIMATION_MODE_ANIMATE_IMAGE;
    }
    else
    {
        return DEVICE_BUSY;
    }

    return DEVICE_OK;
}

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
int AnimatedDisplay::animate(Image image, int delay, int stride, int startingPosition, int autoClear)
{
    //sanitise the delay value
    if(delay <= 0)
        return DEVICE_INVALID_PARAMETER;

    // If there's an ongoing animation, wait for our turn to display.
    this->waitForFreeDisplay();

    // If the display is free, it's our turn to display.
    // If someone called stopAnimation(), then we simply skip...
    if (animationMode == ANIMATION_MODE_NONE)
    {
        // Start the effect.
        this->animateAsync(image, delay, stride, startingPosition, autoClear);

        // Wait for completion.
        //TODO: Put this in when we merge tight-validation
        //if (delay > 0)
            fiberWait();
    }
    else
    {
        return DEVICE_CANCELLED;
    }

    return DEVICE_OK;
}


/**
* Frame update method, invoked periodically to update animations if neccessary.
*/
void AnimatedDisplay::periodicCallback()
{
    this->animationUpdate();
}

/**
* Destructor for AnimatedDisplay, where we deregister this instance from the array of system components.
*/
AnimatedDisplay::~AnimatedDisplay()
{
    status &= ~DEVICE_COMPONENT_STATUS_SYSTEM_TICK;
}
