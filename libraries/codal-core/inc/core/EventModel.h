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

#ifndef EVENT_MODEL_H
#define EVENT_MODEL_H

#include "CodalConfig.h"
#include "CodalComponent.h"
#include "Event.h"
#include "CodalListener.h"
#include "ErrorNo.h"

/**
  * Class definition for the codal device EventModel.
  *
  * It is common to need to send events from one part of a program (or system) to another.
  * The way that these events are stored and delivered is known as an Event Model...
  *
  * The codal device can be programmed in a number of languages, and it not be good to
  * constrain those languages to any particular event model (e.g. they may have their own already).
  *
  * This class defines the functionality an event model needs to have to be able to interact
  * with events generated and/or used by the codal device runtime. Programmer may choose to implement
  * such functionality to integrate their own event models.
  *
  * This is an example of a key principle in computing - ABSTRACTION. This is now part of the
  * UK's Computing curriculum in schools... so ask your teacher about it. :-)
  *
  * An EventModel implementation is provided in the DeviceMessageBus class.
  */
namespace codal
{
    class EventModel
    {
        uint16_t eventHandle;

        protected:

        void (*listener_deletion_callback)(Listener *);         // An optional callback function that is invoked when a listener is removed.

        public:

        static EventModel *defaultEventBus;

        EventModel() : listener_deletion_callback(NULL) {}
        /**
          * Queues the given event to be sent to all registered recipients.
          * The method of delivery will vary depending on the underlying implementation.
          *
          * @param The event to send.
          *
          * @return This default implementation simply returns DEVICE_NOT_SUPPORTED.
          */
        virtual int send(Event)
        {
            return DEVICE_NOT_SUPPORTED;
        }

        /**
         * Add the given Listener to the list of event handlers, unconditionally.
         *
         * @param listener The Listener to validate.
         *
         * @return This default implementation simply returns DEVICE_NOT_SUPPORTED.
         */
        virtual int add(Listener*)
        {
            return DEVICE_NOT_SUPPORTED;
        }

        /**
         * Remove the given Listener from the list of event handlers.
         *
         * @param listener The Listener to remove.
         *
         * @return This default implementation simply returns DEVICE_NOT_SUPPORTED.
         */
        virtual int remove(Listener *)
        {
            return DEVICE_NOT_SUPPORTED;
        }

        /**
          * Returns the Listener at the given position in the list.
          *
          * @param n The index of the desired Listener.
          *
          * @return This default implementation simply returns NULL.
          */
        Listener *elementAt(int)
        {
            return NULL;
        }

        /**
          * Define the default EventModel to use for events raised and consumed by the codal runtime.
          * The default EventModel may be changed at any time.
          *
          * @param model A new instance of an EventModel to use as the default.
          *
          * @return DEVICE_OK on success.
          *
          * Example:
          * @code
          * DeviceMessageBus b();
          * EventModel:setDefaultEventModel(b);
          * @endcode
          */
        static int setDefaultEventModel(EventModel &model)
        {
            EventModel::defaultEventBus = &model;
            return DEVICE_OK;
        }

        /**
         * Sets a pointer to a handler that is invoked when any listener is deleted.
         *
         * @returns DEVICE_OK on success.
         **/
        int setListenerDeletionCallback(void (*listener_deletion_callback)(Listener *))
        {
            this->listener_deletion_callback = listener_deletion_callback;
            return DEVICE_OK;
        }

        /**
          * Register a listener function.
          *
          * An EventModel implementing this interface may optionally choose to override this method,
          * if that EventModel supports asynchronous callbacks to user code, but there is no
          * requirement to do so.
          *
          * @param id The source of messages to listen for. Events sent from any other IDs will be filtered.
          * Use DEVICE_ID_ANY to receive events from all components.
          *
          * @param value The value of messages to listen for. Events with any other values will be filtered.
          * Use DEVICE_EVT_ANY to receive events of any value.
          *
          * @param handler The function to call when an event is received.
          *
          * @param flags User specified, implementation specific flags, that allow behaviour of this events listener
          * to be tuned.
          *
          * @return DEVICE_OK on success, or any valid error code defined in "ErrNo.h". The default implementation
          * simply returns DEVICE_NOT_SUPPORTED.
          *
          * @code
          * void onButtonBClicked(Event)
          * {
          *     //do something
          * }
          *
          * // call onButtonBClicked when ever a click event from buttonB is detected.
          * uBit.messageBus.listen(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          * @endcode
          */
        int listen(int id, int value, void (*handler)(Event), uint16_t flags = EVENT_LISTENER_DEFAULT_FLAGS)
        {
            if (handler == NULL)
                return DEVICE_INVALID_PARAMETER;

            if(id == DEVICE_ID_SCHEDULER && flags != MESSAGE_BUS_LISTENER_IMMEDIATE)
                return DEVICE_INVALID_PARAMETER;

            Listener *newListener = new Listener(id, value, handler, flags);

            if(add(newListener) == DEVICE_OK)
                return DEVICE_OK;

            delete newListener;

            return DEVICE_NOT_SUPPORTED;
        }

        /**
          * Register a listener function.
          *
          * An EventModel implementing this interface may optionally choose to override this method,
          * if that EventModel supports asynchronous callbacks to user code, but there is no
          * requirement to do so.
          *
          * @param id The source of messages to listen for. Events sent from any other IDs will be filtered.
          * Use DEVICE_ID_ANY to receive events from all components.
          *
          * @param value The value of messages to listen for. Events with any other values will be filtered.
          * Use DEVICE_EVT_ANY to receive events of any value.
          *
          * @param handler The function to call when an event is received.
          *
          * @param arg Provide the callback with in an additional argument.
          *
          * @param flags User specified, implementation specific flags, that allow behaviour of this events listener
          * to be tuned.
          *
          * @return DEVICE_OK on success, or any valid error code defined in "ErrNo.h". The default implementation
          * simply returns DEVICE_NOT_SUPPORTED.
          *
          * @code
          * void onButtonBClicked(Event, void* data)
          * {
          *     //do something
          * }
          *
          * // call onButtonBClicked when ever a click event from buttonB is detected.
          * uBit.messageBus.listen(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          * @endcode
          */
        int listen(int id, int value, void (*handler)(Event, void*), void* arg, uint16_t flags = EVENT_LISTENER_DEFAULT_FLAGS)
        {
            if (handler == NULL)
                return DEVICE_INVALID_PARAMETER;

            if(id == DEVICE_ID_SCHEDULER && flags != MESSAGE_BUS_LISTENER_IMMEDIATE)
                return DEVICE_INVALID_PARAMETER;

            Listener *newListener = new Listener(id, value, handler, arg, flags);

            if(add(newListener) == DEVICE_OK)
                return DEVICE_OK;

            delete newListener;

            return DEVICE_NOT_SUPPORTED;
        }

        /**
          * Register a listener function.
          *
          * @param id The source of messages to listen for. Events sent from any other IDs will be filtered.
          * Use DEVICE_ID_ANY to receive events from all components.
          *
          * @param value The value of messages to listen for. Events with any other values will be filtered.
          * Use DEVICE_EVT_ANY to receive events of any value.
          *
          * @param hander The function to call when an event is received.
          *
          * @param flags User specified, implementation specific flags, that allow behaviour of this events listener
          * to be tuned.
          *
          * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler or object
          *         pointers are NULL.
          *
          * @code
          * void SomeClass::onButtonBClicked(Event)
          * {
          *     //do something
          * }
          *
          * SomeClass s = new SomeClass();
          *
          * uBit.messageBus.listen(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, s, &SomeClass::onButtonBClick);
          * @endcode
          */
        template <typename T>
        int listen(uint16_t id, uint16_t value, T* object, void (T::*handler)(Event), uint16_t flags = EVENT_LISTENER_DEFAULT_FLAGS);

        /**
          * Register a listener function.
          *
          * An EventModel implementing this interface may optionally choose to override this method,
          * if that EventModel supports asynchronous callbacks to user code, but there is no
          * requirement to do so.
          *
          * @param component The source component of messages to listen for.
          *
          * @param value The value of messages to listen for. Events with any other values will be filtered.
          * Use DEVICE_EVT_ANY to receive events of any value.
          *
          * @param handler The function to call when an event is received.
          *
          * @param flags User specified, implementation specific flags, that allow behaviour of this events listener
          * to be tuned.
          *
          * @return DEVICE_OK on success, or any valid error code defined in "ErrNo.h". The default implementation
          * simply returns DEVICE_NOT_SUPPORTED.
          *
          * @code
          * void onButtonBClicked(Event)
          * {
          *     //do something
          * }
          *
          * // call onButtonBClicked when ever a click event from buttonB is detected.
          * uBit.messageBus.listen(device.buttonB, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          * @endcode
          */
        int listen(CodalComponent& component, int value, void (*handler)(Event), uint16_t flags = EVENT_LISTENER_DEFAULT_FLAGS)
        {
            return listen(component.id, value, handler, flags);
        }

        /**
          * Register a listener function.
          *
          * An EventModel implementing this interface may optionally choose to override this method,
          * if that EventModel supports asynchronous callbacks to user code, but there is no
          * requirement to do so.
          *
          * @param component The source component of messages to listen for.
          *
          * @param value The value of messages to listen for. Events with any other values will be filtered.
          * Use DEVICE_EVT_ANY to receive events of any value.
          *
          * @param handler The function to call when an event is received.
          *
          * @param arg Provide the callback with in an additional argument.
          *
          * @param flags User specified, implementation specific flags, that allow behaviour of this events listener
          * to be tuned.
          *
          * @return DEVICE_OK on success, or any valid error code defined in "ErrNo.h". The default implementation
          * simply returns DEVICE_NOT_SUPPORTED.
          *
          * @code
          * void onButtonBClicked(Event, void* data)
          * {
          *     //do something
          * }
          *
          * // call onButtonBClicked when ever a click event from buttonB is detected.
          * uBit.messageBus.listen(device.buttonB, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          * @endcode
          */
        int listen(CodalComponent& component, int value, void (*handler)(Event, void*), void* arg, uint16_t flags = EVENT_LISTENER_DEFAULT_FLAGS)
        {
            return listen(component.id, value, handler, arg, flags);
        }

        /**
          * Register a listener function.
          *
          * @param component The source component of messages to listen for.
          *
          * @param value The value of messages to listen for. Events with any other values will be filtered.
          * Use DEVICE_EVT_ANY to receive events of any value.
          *
          * @param hander The function to call when an event is received.
          *
          * @param flags User specified, implementation specific flags, that allow behaviour of this events listener
          * to be tuned.
          *
          * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler or object
          *         pointers are NULL.
          *
          * @code
          * void SomeClass::onButtonBClicked(Event)
          * {
          *     //do something
          * }
          *
          * SomeClass s = new SomeClass();
          *
          * uBit.messageBus.listen(device.buttonB, DEVICE_BUTTON_EVT_CLICK, s, &SomeClass::onButtonBClick);
          * @endcode
          */
        template <typename T>
        int listen(CodalComponent& component, uint16_t value, T* object, void (T::*handler)(Event), uint16_t flags = EVENT_LISTENER_DEFAULT_FLAGS);


        /**
          * Unregister a listener function.
          * Listeners are identified by the Event ID, Event value and handler registered using listen().
          *
          * @param id The Event ID used to register the listener.
          * @param value The Event value used to register the listener.
          * @param handler The function used to register the listener.
          *
          * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler
          *         given is NULL.
          *
          * Example:
          * @code
          * void onButtonBClick(Event)
          * {
          *     //do something
          * }
          *
          * uBit.messageBus.listen(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          *
          * // the previously created listener is now ignored.
          * uBit.messageBus.ignore(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          * @endcode
          */
        int ignore(int id, int value, void (*handler)(Event))
        {
            if (handler == NULL)
                return DEVICE_INVALID_PARAMETER;

            Listener listener(id, value, handler);
            remove(&listener);

            return DEVICE_OK;
        }

        /**
          * Unregister a listener function.
          * Listeners are identified by the Event ID, Event value and handler registered using listen().
          *
          * @param id The Event ID used to register the listener.
          * @param value The Event value used to register the listener.
          * @param handler The function used to register the listener.
          *
          * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler
          *         given is NULL.
          *
          * Example:
          * @code
          * void onButtonBClick(Event, void* data)
          * {
          *     //do something
          * }
          *
          * uBit.messageBus.listen(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          *
          * // the previously created listener is now ignored.
          * uBit.messageBus.ignore(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          * @endcode
          */
        int ignore(int id, int value, void (*handler)(Event, void*))
        {
            if (handler == NULL)
                return DEVICE_INVALID_PARAMETER;

            Listener listener(id, value, handler, NULL);
            remove(&listener);

            return DEVICE_OK;
        }

        /**
          * Unregister a listener function.
          * Listners are identified by the Event ID, Event value and handler registered using listen().
          *
          * @param id The Event ID used to register the listener.
          * @param value The Event value used to register the listener.
          * @param handler The function used to register the listener.
          *
          * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler or object
          *         pointers are NULL.
          *
          * Example:
          * @code
          *
          * void SomeClass::onButtonBClick()
          * {
          *     //do something
          * }
          *
          * SomeClass s = new SomeClass();
          * uBit.messageBus.listen(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, s, &SomeClass::onButtonBClick);
          *
          * // the previously created listener is now ignored.
          * uBit.messageBus.ignore(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, s, &SomeClass::onButtonBClick);
          * @endcode
          */
        template <typename T>
        int ignore(uint16_t id, uint16_t value, T* object, void (T::*handler)(Event));

        /**
          * Unregister a listener function.
          * Listeners are identified by the Event ID, Event value and handler registered using listen().
          *
          * @param component The source component used to register the listener
          * @param value The Event value used to register the listener.
          * @param handler The function used to register the listener.
          *
          * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler
          *         given is NULL.
          *
          * Example:
          * @code
          * void onButtonBClick(Event)
          * {
          *     //do something
          * }
          *
          * uBit.messageBus.listen(device.buttonB, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          *
          * // the previously created listener is now ignored.
          * uBit.messageBus.ignore(device.buttonB, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          * @endcode
          */
        int ignore(CodalComponent& component, int value, void (*handler)(Event))
        {
            return ignore(component.id, value, handler);
        }

        /**
          * Unregister a listener function.
          * Listeners are identified by the Event ID, Event value and handler registered using listen().
          *
          * @param component The source component used to register the listener
          * @param value The Event value used to register the listener.
          * @param handler The function used to register the listener.
          *
          * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler
          *         given is NULL.
          *
          * Example:
          * @code
          * void onButtonBClick(Event, void* data)
          * {
          *     //do something
          * }
          *
          * uBit.messageBus.listen(device.buttonB, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          *
          * // the previously created listener is now ignored.
          * uBit.messageBus.ignore(device.buttonB, DEVICE_BUTTON_EVT_CLICK, onButtonBClick);
          * @endcode
          */
        int ignore(CodalComponent& component, int value, void (*handler)(Event, void*))
        {
            return ignore(component.id, value, handler);
        }

        /**
          * Unregister a listener function.
          * Listners are identified by the Event ID, Event value and handler registered using listen().
          *
          * @param component The source component used to register the listener
          * @param value The Event value used to register the listener.
          * @param handler The function used to register the listener.
          *
          * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler or object
          *         pointers are NULL.
          *
          * Example:
          * @code
          *
          * void SomeClass::onButtonBClick()
          * {
          *     //do something
          * }
          *
          * SomeClass s = new SomeClass();
          * uBit.messageBus.listen(device.buttonB, DEVICE_BUTTON_EVT_CLICK, s, &SomeClass::onButtonBClick);
          *
          * // the previously created listener is now ignored.
          * uBit.messageBus.ignore(device.buttonB, DEVICE_BUTTON_EVT_CLICK, s, &SomeClass::onButtonBClick);
          * @endcode
          */
        template <typename T>
        int ignore(CodalComponent& component, uint16_t value, T* object, void (T::*handler)(Event));

    };

    /**
      * A registration function to allow C++ member functions (methods) to be registered as an event
      * listener.
      *
      * @param id The source of messages to listen for. Events sent from any other IDs will be filtered.
      * Use DEVICE_ID_ANY to receive events from all components.
      *
      * @param value The value of messages to listen for. Events with any other values will be filtered.
      * Use DEVICE_EVT_ANY to receive events of any value.
      *
      * @param object The object on which the method should be invoked.
      *
      * @param handler The method to call when an event is received.
      *
      * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler or object
      *         pointers are NULL.
      */
    template <typename T>
    int EventModel::listen(uint16_t id, uint16_t value, T* object, void (T::*handler)(Event), uint16_t flags)
    {
        if (object == NULL || handler == NULL)
            return DEVICE_INVALID_PARAMETER;

        if(id == DEVICE_ID_SCHEDULER && flags != MESSAGE_BUS_LISTENER_IMMEDIATE)
            return DEVICE_INVALID_PARAMETER;

        Listener *newListener = new Listener(id, value, object, handler, flags);

        if(add(newListener) == DEVICE_OK)
            return DEVICE_OK;

        delete newListener;
        return DEVICE_NOT_SUPPORTED;
    }

    /**
      * A registration function to allow C++ member functions (methods) to be registered as an event
      * listener.
      *
      * @param component The source component of messages to listen for.
      *
      * @param value The value of messages to listen for. Events with any other values will be filtered.
      * Use DEVICE_EVT_ANY to receive events of any value.
      *
      * @param object The object on which the method should be invoked.
      *
      * @param handler The method to call when an event is received.
      *
      * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler or object
      *         pointers are NULL.
      */
    template <typename T>
    int EventModel::listen(CodalComponent& component, uint16_t value, T* object, void (T::*handler)(Event), uint16_t flags)
    {
        return listen(component.id, value, object, handler, flags);
    }

    /**
      * Unregister a listener function.
      * Listners are identified by the Event ID, Event value and handler registered using listen().
      *
      * @param id The Event ID used to register the listener.
      * @param value The Event value used to register the listener.
      * @param handler The function used to register the listener.
      *
      * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler or object
      *         pointers are NULL.
      *
      * Example:
      * @code
      *
      * void SomeClass::onButtonBClick()
      * {
      *     //do something
      * }
      *
      * SomeClass s = new SomeClass();
      * uBit.messageBus.listen(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, s, &SomeClass::onButtonBClick);
      *
      * // the previously created listener is now ignored.
      * uBit.messageBus.ignore(DEVICE_ID_BUTTON_B, DEVICE_BUTTON_EVT_CLICK, s, &SomeClass::onButtonBClick);
      * @endcode
      */
    template <typename T>
    int EventModel::ignore(uint16_t id, uint16_t value, T* object, void (T::*handler)(Event))
    {
        if (handler == NULL)
            return DEVICE_INVALID_PARAMETER;

        Listener listener(id, value, object, handler);
        remove(&listener);

        return DEVICE_OK;
    }

    /**
      * Unregister a listener function.
      * Listners are identified by the Event ID, Event value and handler registered using listen().
      *
      * @param component The source component used to register the listener
      * @param value The Event value used to register the listener.
      * @param handler The function used to register the listener.
      *
      * @return DEVICE_OK on success or DEVICE_INVALID_PARAMETER if the handler or object
      *         pointers are NULL.
      *
      * Example:
      * @code
      *
      * void SomeClass::onButtonBClick()
      * {
      *     //do something
      * }
      *
      * SomeClass s = new SomeClass();
      * uBit.messageBus.listen(device.buttonB, DEVICE_BUTTON_EVT_CLICK, s, &SomeClass::onButtonBClick);
      *
      * // the previously created listener is now ignored.
      * uBit.messageBus.ignore(device.buttonB, DEVICE_BUTTON_EVT_CLICK, s, &SomeClass::onButtonBClick);
      * @endcode
      */
    template <typename T>
    int EventModel::ignore(CodalComponent& component, uint16_t value, T* object, void (T::*handler)(Event))
    {
        return ignore(component.id, value, object, handler);
    }
}

#endif
