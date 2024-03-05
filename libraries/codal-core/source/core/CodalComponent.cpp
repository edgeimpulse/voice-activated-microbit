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

#include "CodalComponent.h"
#include "CodalFiber.h"
#include "EventModel.h"
#include "Timer.h"

using namespace codal;

CodalComponent* CodalComponent::components[DEVICE_COMPONENT_COUNT];

uint8_t CodalComponent::configuration = 0;

#if DEVICE_COMPONENT_COUNT > 255
#error "DEVICE_COMPONENT_COUNT has to fit in uint8_t"
#endif

/**
  * The periodic callback for all components.
  */
void component_callback(Event evt)
{
    uint8_t i = 0;

    if(evt.value == DEVICE_COMPONENT_EVT_SYSTEM_TICK)
    {
        while(i < DEVICE_COMPONENT_COUNT)
        {
            if(CodalComponent::components[i] && CodalComponent::components[i]->status & DEVICE_COMPONENT_STATUS_SYSTEM_TICK)
                CodalComponent::components[i]->periodicCallback();

            i++;
        }
    }

    if(evt.value == DEVICE_SCHEDULER_EVT_IDLE)
    {
        while(i < DEVICE_COMPONENT_COUNT)
        {
            if(CodalComponent::components[i] && CodalComponent::components[i]->status & DEVICE_COMPONENT_STATUS_IDLE_TICK)
                CodalComponent::components[i]->idleCallback();

            i++;
        }
    }
}

/**
  * Adds the current CodalComponent instance to our array of components.
  */
void CodalComponent::addComponent()
{
    uint8_t i = 0;

    // iterate through our list until an empty space is found.
    while(i < DEVICE_COMPONENT_COUNT)
    {
        if(components[i] == NULL)
        {
            components[i] = this;
            break;
        }

        i++;
    }

    if(!(configuration & DEVICE_COMPONENT_LISTENERS_CONFIGURED) && EventModel::defaultEventBus)
    {
        int ret = system_timer_event_every_us(SCHEDULER_TICK_PERIOD_US, DEVICE_ID_COMPONENT, DEVICE_COMPONENT_EVT_SYSTEM_TICK);

        if(ret == DEVICE_OK)
        {
            EventModel::defaultEventBus->listen(DEVICE_ID_COMPONENT, DEVICE_COMPONENT_EVT_SYSTEM_TICK, component_callback, MESSAGE_BUS_LISTENER_IMMEDIATE);
            EventModel::defaultEventBus->listen(DEVICE_ID_SCHEDULER, DEVICE_SCHEDULER_EVT_IDLE, component_callback, MESSAGE_BUS_LISTENER_IMMEDIATE);

            CodalComponent::configuration |= DEVICE_COMPONENT_LISTENERS_CONFIGURED;
        }
    }
}

/**
  * Removes the current CodalComponent instance from our array of components.
  */
void CodalComponent::removeComponent()
{
    uint8_t i = 0;

    while(i < DEVICE_COMPONENT_COUNT)
    {
        if(components[i] == this)
        {
            components[i] = NULL;
            return;
        }

        i++;
    }
}

/**
 * Puts all components in (or out of) sleep (low power) mode.
 */
void CodalComponent::setAllSleep(bool doSleep)
{
    // usually, dependencies of component X are added before X itself,
    // so iterate backwards (so from high-level components to low-level)
    // when putting stuff to sleep, and forwards when waking up
    if (doSleep)
    {
        for (int i = DEVICE_COMPONENT_COUNT - 1; i >= 0; i--)
            if (components[i])
                components[i]->setSleep(true);
    }
    else
    {
        for (unsigned i = 0; i < DEVICE_COMPONENT_COUNT; i++)
            if (components[i])
                components[i]->setSleep(false);
    }
}
