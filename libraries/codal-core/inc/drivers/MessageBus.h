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

#ifndef DEVICE_MESSAGE_BUS_H
#define DEVICE_MESSAGE_BUS_H

#include "CodalConfig.h"
#include "CodalComponent.h"
#include "Event.h"
#include "CodalListener.h"
#include "EventModel.h"


namespace codal
{
    /**
      * Class definition for the MessageBus.
      *
      * The MessageBus is the common mechanism to deliver asynchronous events on the
      * Device platform. It serves a number of purposes:
      *
      * 1) It provides an eventing abstraction that is independent of the underlying substrate.
      *
      * 2) It provides a mechanism to decouple user code from trusted system code
      *    i.e. the basis of a message passing nano kernel.
      *
      * 3) It allows a common high level eventing abstraction across a range of hardware types.e.g. buttons, BLE...
      *
      * 4) It provides a mechanim for extensibility - new devices added via I/O pins can have OO based
      *    drivers and communicate via the message bus with minima impact on user level languages.
      *
      * 5) It allows for the possiblility of event / data aggregation, which in turn can save energy.
      *
      * It has the following design principles:
      *
      * 1) Maintain a low RAM footprint where possible
      *
      * 2) Make few assumptions about the underlying platform, but allow optimizations where possible.
      */
    class MessageBus : public EventModel, public CodalComponent
    {
        uint16_t        userNotifyId;

        public:

        /**
          * Default constructor.
          *
          * Adds itself as a fiber component, and also configures itself to be the
          * default EventModel if defaultEventBus is NULL.
          */
        MessageBus();

        /**
          * Queues the given event to be sent to all registered recipients.
          *
          * @param evt The event to send.
          *
          * @code
          * MessageBus bus;
          *
          * // Creates and sends the Event using bus.
          * Event evt(DEVICE_ID_BUTTON_A, DEVICE_BUTTON_EVT_CLICK);
          *
          * // Creates the Event, but delays the sending of that event.
          * Event evt1(DEVICE_ID_BUTTON_A, DEVICE_BUTTON_EVT_CLICK, CREATE_ONLY);
          *
          * bus.send(evt1);
          *
          * // This has the same effect!
          * evt1.fire()
          * @endcode
          */
        virtual int send(Event evt);

        /**
          * Internal function, used to deliver the given event to all relevant recipients.
          * Normally, this is called once an event has been removed from the event queue.
          *
          * @param evt The event to send.
          *
          * @param urgent The type of listeners to process (optional). If set to true, only listeners defined as urgent and non-blocking will be processed
          *               otherwise, all other (standard) listeners will be processed. Defaults to false.
          *
          * @return 1 if all matching listeners were processed, 0 if further processing is required.
          *
          * @note It is recommended that all external code uses the send() function instead of this function,
          *       or the constructors provided by Event.
          */
        int process(Event &evt, bool urgent = false);

        /**
          * Returns the Listener with the given position in our list.
          *
          * @param n The position in the list to return.
          *
          * @return the Listener at postion n in the list, or NULL if the position is invalid.
          */
        virtual Listener *elementAt(int n);

        /**
          * Destructor for MessageBus, where we deregister this instance from the array of fiber components.
          */
        ~MessageBus();

        /**
          * Add the given Listener to the list of event handlers, unconditionally.
          *
          * @param listener The Listener to add.
          *
          * @return DEVICE_OK if the listener is valid, DEVICE_INVALID_PARAMETER otherwise.
          */
        virtual int add(Listener *newListener);

        /**
          * Remove the given Listener from the list of event handlers.
          *
          * @param listener The Listener to remove.
          *
          * @return DEVICE_OK if the listener is valid, DEVICE_INVALID_PARAMETER otherwise.
          */
        virtual int remove(Listener *newListener);



        private:

        Listener            *listeners;           // Chain of active listeners.
        EventQueueItem      *evt_queue_head;    // Head of queued events to be processed.
        EventQueueItem      *evt_queue_tail;    // Tail of queued events to be processed.
        uint16_t                    nonce_val;          // The last nonce issued.
        uint16_t                    queueLength;        // The number of events currently waiting to be processed.

        /**
          * Cleanup any Listeners marked for deletion from the list.
          *
          * @return The number of listeners removed from the list.
          */
        int deleteMarkedListeners();

        /**
          * Queue the given event for processing at a later time.
          * Add the given event at the tail of our queue.
          *
          * @param The event to queue.
          */
        void queueEvent(Event &evt);

        /**
          * Extract the next event from the front of the event queue (if present).
          *
          * @return a pointer to the EventQueueItem that is at the head of the list.
          */
        EventQueueItem* dequeueEvent();

        /**
          * Periodic callback from Device.
          *
          * Process at least one event from the event queue, if it is not empty.
          * We then continue processing events until something appears on the runqueue.
          */
        void idle(Event);
    };

    /**
     * Allocate a NOTIFY event code dynamicaly, for generally purpose condition synchronisation.
     */
    uint16_t allocateNotifyEvent();
}

#endif
