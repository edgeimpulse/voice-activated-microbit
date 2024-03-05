#ifndef LOW_LEVEL_TIMER_H
#define LOW_LEVEL_TIMER_H

#include "CodalConfig.h"
#include "CodalComponent.h"
#include "codal_target_hal.h"

namespace codal
{

/**
 * An enumeration that represents the current mode of the timer.
 **/
enum TimerMode
{
    TimerModeTimer = 0,
    TimerModeCounter,
    TimerModeAlternateFunction
};

/**
 * An enumeration that represents the number of bits (i.e. the top) used to count.
 *
 * This enumeration is used to compute roll over calculations and must be accurate.
 **/
enum TimerBitMode
{
    BitMode8 = 0,
    BitMode16,
    BitMode24,
    BitMode32
};

/**
 * This class represents a timer in its rawest form, it allows direct manipulation of timer registers through a common API.
 *
 * Higher level drivers can then use / layer on top of the low level timer interface.
 **/
class LowLevelTimer : public CodalComponent
{
    protected:
    TimerBitMode bitMode; // the current bitMode of the timer.
    uint8_t channel_count; // the number of channels this timer instance has.

    public:

    /**
     * A function pointer that it is invoked from the Low Level Timer interrupt context.
     *
     * @param channel_bitmsk A 16 bit number that represents the channels that have triggered a match. I.e. if CC register 0 is set, bit 0 will be set to one.
     **/
    void (*timer_pointer) (uint16_t channel_bitmsk);

    /**
     * Sets the timer_pointer member variable.
     *
     * @returns DEVICE_OK on success.
     **/
    virtual int setIRQ(void (*timer_pointer) (uint16_t channel_bitmsk))
    {
        this->timer_pointer = timer_pointer;
        return DEVICE_OK;
    }

    /**
     * Sets the interrupt priority (if supported on this mcu)
     *
     * @returns DEVICE_OK on success.
     **/
    virtual int setIRQPriority(int)
    {
        target_panic(DEVICE_NOT_IMPLEMENTED);
        return DEVICE_NOT_IMPLEMENTED;
    }

    /**
     * Constructor
     *
     * @param channel_count the number of capture compare registers the underlying hardware has.
     **/
    LowLevelTimer(uint8_t channel_count)
    {
        this->channel_count = channel_count;
        this->timer_pointer = NULL;
    }

    /**
     * Destructor
     **/
    virtual ~LowLevelTimer()
    {
    }

    /**
     * Enables this timer instance and begins counting
     **/
    virtual int enable() = 0;

    /**
     * Enables the IRQ of this timer instance
     **/
    virtual int enableIRQ() = 0;

    /**
     * Disables this timer instance and stops counting
     **/
    virtual int disable() = 0;

    /**
     * Disables the IRQ of this timer instance
     **/
    virtual int disableIRQ() = 0;

    /**
     * Resets the counter of this timer.
     **/
    virtual int reset() = 0;

    /**
     * Sets the mode of the timer
     *
     * @param t the timer mode to use. Underlying hardware should be configured appropriately.
     **/
    virtual int setMode(TimerMode t) = 0;

    /**
     * Sets the compare value of a capture compare register in the underlying hardware
     *
     * @param channel the channel to load the value into.
     *
     * @param value the value to load into the capture compare register
     **/
    virtual int setCompare(uint8_t channel, uint32_t value)= 0;

    /**
     * Offsets the compare value of a capture compare register in the underlying hardware
     *
     * @param channel the channel to offset by value.
     *
     * @param value the value to offset the capture compare register by.
     **/
    virtual int offsetCompare(uint8_t channel, uint32_t value) = 0;

    /**
     * Disables the interrupt for the given channel, and sets the value to 0.
     *
     * @param channel the channel to clear
     **/
    virtual int clearCompare(uint8_t channel) = 0;

    /**
     * Returns the counter value of the underlying hardware.
     **/
    virtual uint32_t captureCounter() = 0;

    /**
     * Sets the frequency of the timer based on a speed given in Khz.
     *
     * @param speedKHz the speed of the timer in KHz.
     **/
    virtual int setClockSpeed(uint32_t speedKHz) = 0;

    /**
     * Sets the resolution of the timer counter.
     *
     * @param t the TimerBitMode value to use.
     **/
    virtual int setBitMode(TimerBitMode t) = 0;

    /**
     * Returns the current bit mode of the timer.
     **/
    virtual TimerBitMode getBitMode()
    {
        return bitMode;
    }

    /**
     * Returns the number of channels this timer has for use.
     **/
    int getChannelCount()
    {
        return channel_count;
    }
};
}


#endif
