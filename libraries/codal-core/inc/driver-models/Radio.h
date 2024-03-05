#ifndef CODAL_RADIO_H
#define CODAL_RADIO_H

#include "CodalComponent.h"
#include "ManagedBuffer.h"

#define RADIO_EVT_DATA_READY        2

namespace codal
{
    class Radio : public CodalComponent
    {
        public:

        Radio(uint16_t id = DEVICE_ID_RADIO)
        {
            this->id = id;
        }

        /**
        * Initialises the radio for use as a multipoint sender/receiver
        *
        * @return DEVICE_OK on success, DEVICE_NOT_SUPPORTED if the BLE stack is running.
        */
        virtual int enable() = 0;

        /**
        * Disables the radio for use as a multipoint sender/receiver.
        *
        * @return DEVICE_OK on success, DEVICE_NOT_SUPPORTED if the BLE stack is running.
        */
        virtual int disable() = 0;

        /**
        * Transmits the given buffer onto the broadcast radio.
        * The call will wait until the transmission of the packet has completed before returning.
        *
        * @param data The packet contents to transmit.
        *
        * @return DEVICE_OK on success, or DEVICE_NOT_SUPPORTED if the BLE stack is running.
        */
        virtual ManagedBuffer recvBuffer() = 0;

        /**
        * Transmits the given buffer onto the broadcast radio.
        * The call will wait until the transmission of the packet has completed before returning.
        *
        * @param data The packet contents to transmit.
        *
        * @return DEVICE_OK on success, or DEVICE_NOT_SUPPORTED if the BLE stack is running.
        */
        virtual int sendBuffer(ManagedBuffer) = 0;
    };
}

#endif