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
  * Definitions for the Device system timer.
  *
  * This module provides:
  *
  * 1) a concept of global system time since power up
  * 2) a simple periodic multiplexing API for the underlying mbed implementation.
  *
  * The latter is useful to avoid costs associated with multiple mbed Ticker instances
  * in codal components, as each incurs a significant additional RAM overhead (circa 80 bytes).
  */

#include "Timer.h"
#include "Event.h"
#include "CodalCompat.h"
#include "ErrorNo.h"
#include "codal_target_hal.h"
#include "CodalDmesg.h"

using namespace codal;

//
// Default system wide timer, if created.
//
Timer* codal::system_timer = NULL;
static uint32_t cycleScale = 0;

void timer_callback(uint16_t chan)
{
    if (system_timer)
    {
        if (chan & (1 << system_timer->ccPeriodChannel))
            system_timer->trigger(true);
        else
            system_timer->trigger(false);
    }
}

void Timer::triggerIn(CODAL_TIMESTAMP t)
{
    if (t < CODAL_TIMER_MINIMUM_PERIOD) t = CODAL_TIMER_MINIMUM_PERIOD;
    // Just in case, disable all IRQs
    target_disable_irq();
    timer.setCompare(this->ccEventChannel, timer.captureCounter() + t);
    target_enable_irq();
}

TimerEvent *Timer::getTimerEvent()
{
    // Find the first unused slot, and assign it.
    for (int i=0; i < eventListSize; i++)
    {
        if (timerEventList[i].id == 0)
            return &timerEventList[i];
    }

    // TODO: should try to realloc the list here.
    return NULL;
}

void Timer::releaseTimerEvent(TimerEvent *event)
{
    event->id = 0;
    if (nextTimerEvent == event)
        nextTimerEvent = NULL;
}

/**
 * Constructor for a generic system clock interface.
 */
Timer::Timer(LowLevelTimer& t, uint8_t ccPeriodChannel, uint8_t ccEventChannel) : timer(t)
{
    // Register ourselves as the defualt timer - most recent timer wins.
    system_timer = this;

    this->ccPeriodChannel = ccPeriodChannel;
    this->ccEventChannel = ccEventChannel;

    // Create an empty event list of the default size.
    eventListSize = CODAL_TIMER_DEFAULT_EVENT_LIST_SIZE;
    timerEventList = (TimerEvent *) malloc(sizeof(TimerEvent) * CODAL_TIMER_DEFAULT_EVENT_LIST_SIZE);
    memclr(timerEventList, sizeof(TimerEvent) * CODAL_TIMER_DEFAULT_EVENT_LIST_SIZE);
    nextTimerEvent = NULL;

    // Reset clock
    currentTime = 0;
    currentTimeUs = 0;

    timer.setIRQ(timer_callback);
    timer.setCompare(ccPeriodChannel, 10000000);
    timer.enable();

    delta = 0;
    sigma = timer.captureCounter();

    system_timer_calibrate_cycles();
}

/**
 * Retrieves the current time tracked by this Timer instance
 * in milliseconds
 *
 * @return the timestamp in milliseconds
 */
CODAL_TIMESTAMP Timer::getTime()
{
    sync();
    return currentTime;
}

/**
 * Retrieves the current time tracked by this Timer instance
 * in microseconds
 *
 * @return the timestamp in microseconds
 */
CODAL_TIMESTAMP Timer::getTimeUs()
{
    sync();
    return currentTimeUs;
}

int Timer::disableInterrupts()
{
    timer.disableIRQ();
    return DEVICE_OK;
}

int Timer::enableInterrupts()
{
    timer.enableIRQ();
    return DEVICE_OK;
}

int Timer::setEvent(CODAL_TIMESTAMP period, uint16_t id, uint16_t value, bool repeat)
{
    TimerEvent *evt = getTimerEvent();
    if (evt == NULL)
        return DEVICE_NO_RESOURCES;

    evt->set(getTimeUs() + period, repeat ? period: 0, id, value);

    target_disable_irq();

    if (nextTimerEvent == NULL || evt->timestamp < nextTimerEvent->timestamp)
    {
        nextTimerEvent = evt;
        triggerIn(period);
    }

    target_enable_irq();

    return DEVICE_OK;
}


/**
 * Cancels any events matching the given id and value.
 *
 * @param id the ID that was given upon a previous call to eventEvery / eventAfter
 *
 * @param value the value that was given upon a previous call to eventEvery / eventAfter
 */
int Timer::cancel(uint16_t id, uint16_t value)
{
    int res = DEVICE_INVALID_PARAMETER;

    target_disable_irq();
    if (nextTimerEvent && nextTimerEvent->id == id && nextTimerEvent->value == value)
    {
        nextTimerEvent->id = 0;
        recomputeNextTimerEvent();
        res = DEVICE_OK;
    }
    else
        for (int i=0; i<eventListSize; i++)
        {
            if (timerEventList[i].id == id && timerEventList[i].value == value)
            {
                timerEventList[i].id = 0;
                res = DEVICE_OK;
                break;
            }
        }
    target_enable_irq();

    return res;
}

/**
 * Configures this Timer instance to fire an event after period
 * milliseconds.
 *
 * @param period the period to wait until an event is triggered, in milliseconds.
 *
 * @param id the ID to be used in event generation.
 *
 * @param value the value to place into the Events' value field.
 */
int Timer::eventAfter(CODAL_TIMESTAMP period, uint16_t id, uint16_t value)
{
    return eventAfterUs(period*1000, id, value);
}

/**
 * Configures this Timer instance to fire an event after period
 * microseconds.
 *
 * @param period the period to wait until an event is triggered, in microseconds.
 *
 * @param id the ID to be used in event generation.
 *
 * @param value the value to place into the Events' value field.
 */
int Timer::eventAfterUs(CODAL_TIMESTAMP period, uint16_t id, uint16_t value)
{
    return setEvent(period, id, value, false);
}

/**
 * Configures this Timer instance to fire an event every period
 * milliseconds.
 *
 * @param period the period to wait until an event is triggered, in milliseconds.
 *
 * @param id the ID to be used in event generation.
 *
 * @param value the value to place into the Events' value field.
 */
int Timer::eventEvery(CODAL_TIMESTAMP period, uint16_t id, uint16_t value)
{
    return eventEveryUs(period*1000, id, value);
}

/**
 * Configures this Timer instance to fire an event every period
 * microseconds.
 *
 * @param period the period to wait until an event is triggered, in microseconds.
 *
 * @param id the ID to be used in event generation.
 *
 * @param value the value to place into the Events' value field.
 */
int Timer::eventEveryUs(CODAL_TIMESTAMP period, uint16_t id, uint16_t value)
{
    return setEvent(period, id, value, true);
}

/**
 * Callback from physical timer implementation code.
 * @param t Indication that t time units (typically microsends) have elapsed.
 */
void Timer::sync()
{
    // Need to disable all IRQs - for example if SPI IRQ is triggered during
    // sync(), it might call into getTimeUs(), which would call sync()
    target_disable_irq();

    uint32_t val = timer.captureCounter();
    uint32_t elapsed = 0;

    // assume at least 16 bit counter; note that this also works when the timer overflows
    elapsed = (uint16_t)(val - sigma);
    sigma = val;

    // advance main timer
    currentTimeUs += elapsed;

    // the 64 bit division is ~150 cycles
    // this is called at least every few ms, and quite possibly much more often
    delta += elapsed;
    while (delta >= 1000) {
        currentTime++;
        delta -= 1000;
    }

    target_enable_irq();
}

void Timer::recomputeNextTimerEvent()
{
    nextTimerEvent = NULL;

    TimerEvent *e = timerEventList;

    // Find the next most recent and schedule it.
    for (int i = 0; i < eventListSize; i++)
    {
        if (e->id != 0 && (nextTimerEvent == NULL || (e->timestamp < nextTimerEvent->timestamp)))
            nextTimerEvent = e;
        e++;
    }

    if (nextTimerEvent) {
        // this may possibly happen if a new timer event was added to the queue while
        // we were running - it might be already in the past
        triggerIn(max(nextTimerEvent->timestamp - currentTimeUs, CODAL_TIMER_MINIMUM_PERIOD));
    }
}

/**
 * Callback from physical timer implementation code.
 */
void Timer::trigger(bool isFallback)
{
    if (isFallback)
        timer.setCompare(ccPeriodChannel, timer.captureCounter() + 10000000);

    int eventsFired;

    sync();

    // Now, walk the list and trigger any events that are pending.
    do
    {
        eventsFired = 0;
        TimerEvent *e = timerEventList;

        for (int i = 0; i<eventListSize; i++)
        {
            if (e->id != 0 && currentTimeUs >= e->timestamp)
            {
                uint16_t id = e->id;
                uint16_t value = e->value;

                // Release before triggering event. Otherwise, an immediate event handler
                // can cancel this event, another event might be put in its place
                // and we end up releasing (or repeating) a completely different event.
                if (e->period == 0)
                    releaseTimerEvent(e);
                else
                    e->timestamp += e->period;

                // We need to trigger this event.
#if CONFIG_ENABLED(LIGHTWEIGHT_EVENTS)
                Event evt(id, value, currentTime);
#else
                Event evt(id, value, currentTimeUs);
#endif

                // TODO: Handle rollover case above...
                eventsFired++;
            }
            e++;
        }

    } while (eventsFired);

    // always recompute nextTimerEvent - event firing could have added new timer events
    recomputeNextTimerEvent();
}

/**
 * Destructor for this Timer instance
 */
Timer::~Timer()
{
}


/*
 *
 * Convenience C API Interface that wraps this class, using the first compatible timer that is created
 *
 */
/**
  * Determines the time since the device was powered on.
  *
  * @return the current time since power on in milliseconds
  */
CODAL_TIMESTAMP codal::system_timer_current_time()
{
    if(system_timer == NULL)
        return 0;

    return system_timer->getTime();
}

/**
  * Determines the time since the device was powered on.
  *
  * @return the current time since power on in microseconds
  */
CODAL_TIMESTAMP codal::system_timer_current_time_us()
{
    if(system_timer == NULL)
        return 0;

    return system_timer->getTimeUs();
}

/**
  * Configure an event to occur every period us.
  *
  * @param period the interval between events
  *
  * @param the value to fire against the current system_timer id.
  *
  * @return DEVICE_OK or DEVICE_NOT_SUPPORTED if no timer has been registered.
  */
int codal::system_timer_event_every_us(CODAL_TIMESTAMP period, uint16_t id, uint16_t value)
{
    if(system_timer == NULL)
        return DEVICE_NOT_SUPPORTED;

    return system_timer->eventEveryUs(period, id, value);
}

/**
  * Configure an event to occur after period us.
  *
  * @param period the interval between events
  *
  * @param the value to fire against the current system_timer id.
  *
  * @return DEVICE_OK or DEVICE_NOT_SUPPORTED if no timer has been registered.
  */
int codal::system_timer_event_after_us(CODAL_TIMESTAMP period, uint16_t id, uint16_t value)
{
    if(system_timer == NULL)
        return DEVICE_NOT_SUPPORTED;

    return system_timer->eventAfterUs(period, id, value);
}

/**
  * Configure an event to occur every period milliseconds.
  *
  * @param period the interval between events
  *
  * @param the value to fire against the current system_timer id.
  *
  * @return DEVICE_OK or DEVICE_NOT_SUPPORTED if no timer has been registered.
  */
int codal::system_timer_event_every(CODAL_TIMESTAMP period, uint16_t id, uint16_t value)
{
    if(system_timer == NULL)
        return DEVICE_NOT_SUPPORTED;

    return system_timer->eventEvery(period, id, value);
}

/**
  * Configure an event to occur after period millseconds.
  *
  * @param period the interval between events
  *
  * @param the value to fire against the current system_timer id.
  *
  * @return DEVICE_OK or DEVICE_NOT_SUPPORTED if no timer has been registered.
  */
int codal::system_timer_event_after(CODAL_TIMESTAMP period, uint16_t id, uint16_t value)
{
    if(system_timer == NULL)
        return DEVICE_NOT_SUPPORTED;

    return system_timer->eventAfter(period, id, value);
}

/**
 * Cancels any events matching the given id and value.
 *
 * @param id the ID that was given upon a previous call to eventEvery / eventAfter
 *
 * @param value the value that was given upon a previous call to eventEvery / eventAfter
 */
int codal::system_timer_cancel_event(uint16_t id, uint16_t value)
{
    if(system_timer == NULL)
        return DEVICE_NOT_SUPPORTED;

    return system_timer->cancel(id, value);
}

/**
 * An auto calibration method that uses the hardware timer to compute the number of cycles
 * per us.
 *
 * The result of this method is then used to compute accurate us waits using instruction counting in system_timer_wait_us.
 *
 * If this method is not called, a less accurate timer implementation is used in system_timer_wait_us.
 *
 * @return DEVICE_OK on success, and DEVICE_NOT_SUPPORTED if no system_timer yet exists.
 */
int codal::system_timer_calibrate_cycles()
{
    if(system_timer == NULL)
        return DEVICE_NOT_SUPPORTED;

    uint32_t start = system_timer->getTimeUs();
    system_timer_wait_cycles(10000);
    uint32_t end = system_timer->getTimeUs();
    cycleScale = (10000) / (end - start - 5);

    return DEVICE_OK;
}

/**
 * Spin wait for a given number of cycles.
 *
 * @param cycles the number of nops to execute
 * @return DEVICE_OK
 */
void codal::system_timer_wait_cycles(uint32_t cycles)
{
    __asm__ __volatile__(
        ".syntax unified\n"
        "1:              \n"
        "   subs %0, #1   \n" // subtract 1 from %0 (n)
        "   bne 1b       \n" // if result is not 0 jump to 1
        : "+r" (cycles)           // '%0' is n variable with RW constraints
        :                    // no input
        :                    // no clobber
    );
}

/**
 * Spin wait using the timer for a given number of microseconds.
 *
 * @param period the period to wait for.
 *
 * @return DEVICE_OK or DEVICE_NOT_SUPPORTED if no timer has been registered.
 *
 * @note this provides a good starting point for non-timing critical applications. For more accurate timings,
 * please use a cycle-based wait approach (see system_timer_wait_cycles)
 */
int codal::system_timer_wait_us(uint32_t period)
{
    if(system_timer == NULL)
        return DEVICE_NOT_SUPPORTED;

    if(cycleScale)
        system_timer_wait_cycles(period * cycleScale);
    else
    {
        CODAL_TIMESTAMP start = system_timer->getTimeUs();
        while(system_timer->getTimeUs() < start + period);
    }

    return DEVICE_OK;
}

/**
 * Spin wait for a given number of milliseconds.
 *
 * @param period the interval between events
 * @return DEVICE_OK or DEVICE_NOT_SUPPORTED if no timer has been registered.
 * @note this provides a good starting point for non-timing critical applications. For more accurate timings,
 * please use a cycle-based wait approach (see system_timer_wait_cycles)
 */
int codal::system_timer_wait_ms(uint32_t period)
{
    return system_timer_wait_us(period * 1000);
}
