#ifndef NRF_TIMER_H
#define NRF_TIMER_H

#include "LowLevelTimer.h"
#include "nrf.h"

#define TIMER_CHANNEL_COUNT              4
namespace codal
{
    class NRFLowLevelTimer : public LowLevelTimer
    {
        IRQn_Type irqn;

        public:
        NRF_TIMER_Type *timer;

        NRFLowLevelTimer(NRF_TIMER_Type* timer, IRQn_Type irqn);

        virtual int setIRQPriority(int priority) override;

        virtual int enable();

        virtual int enableIRQ();

        virtual int disable();

        virtual int disableIRQ();

        virtual int reset();

        virtual int setMode(TimerMode t);

        virtual int setCompare(uint8_t channel, uint32_t value);

        virtual int offsetCompare(uint8_t channel, uint32_t value);

        virtual int clearCompare(uint8_t channel);

        virtual uint32_t captureCounter();

        virtual int setClockSpeed(uint32_t speedKHz);

        virtual int setBitMode(TimerBitMode t);

        virtual int setSleep(bool doSleep) override;
    };
}

#endif