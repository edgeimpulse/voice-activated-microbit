/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

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

#include "CodalConfig.h"
#include "NRF52Radio.h"
#include "CodalDmesg.h"

using namespace codal;

/**
  * Provides a simple broadcast radio abstraction, built upon the raw nrf51822 RADIO module.
  *
  * This class provides the ability to broadcast simple text or binary messages to other micro:bits in the vicinity
  * It is envisaged that this would provide the basis for children to experiment with building their own, simple,
  * custom protocols.
  *
  * @note This API does not contain any form of encryption, authentication or authorisation. Its purpose is solely for use as a
  * teaching aid to demonstrate how simple communications operates, and to provide a sandpit through which learning can take place.
  * For serious applications, BLE should be considered a substantially more secure alternative.
  */

/**
* Constructor.
*
* Creates an instance of a NRF52RadioDatagram which offers the ability
* to broadcast simple text or binary messages to other micro:bits in the vicinity
*
* @param r The underlying radio module used to send and receive data.
*/
NRF52RadioDatagram::NRF52RadioDatagram(NRF52Radio &r) : radio(r)
{
    this->rxQueue = NULL;
}

/**
  * Retrieves packet payload data into the given buffer.
  *
  * If a data packet is already available, then it will be returned immediately to the caller.
  * If no data is available then DEVICE_INVALID_PARAMETER is returned.
  *
  * @param buf A pointer to a valid memory location where the received data is to be stored
  *
  * @param len The maximum amount of data that can safely be stored in 'buf'
  *
  * @return The length of the data stored, or DEVICE_INVALID_PARAMETER if no data is available, or the memory regions provided are invalid.
  */
int NRF52RadioDatagram::recv(uint8_t *buf, int len)
{
    if (buf == NULL || rxQueue == NULL || len < 0)
        return DEVICE_INVALID_PARAMETER;

    // Take the first buffer from the queue.
    FrameBuffer *p = rxQueue;
    rxQueue = rxQueue->next;

    int l = min(len, p->length - (NRF52_RADIO_HEADER_SIZE - 1));

    // Fill in the buffer provided, if possible.
    memcpy(buf, p->payload, l);

    delete p;
    return l;
}

/**
  * Retreives packet payload data into the given buffer.
  *
  * If a data packet is already available, then it will be returned immediately to the caller
  * in the form of a ManagedBuffer.
  *
  * @return the data received, or an empty ManagedBuffer if no data is available.
  */
ManagedBuffer NRF52RadioDatagram::recv()
{
    if (rxQueue == NULL)
    {
        DMESG("RXQ EMPTY");
        return ManagedBuffer();
    }

    FrameBuffer *p = rxQueue;
    rxQueue = rxQueue->next;

    DMESG("MAKING buff: %d", p->length - (NRF52_RADIO_HEADER_SIZE - 1));
    ManagedBuffer packet(p->payload, p->length - (NRF52_RADIO_HEADER_SIZE - 1));
    DMESG("DONE");
    delete p;
    return packet;
}

/**
  * Transmits the given buffer onto the broadcast radio.
  *
  * This is a synchronous call that will wait until the transmission of the packet
  * has completed before returning.
  *
  * @param buffer The packet contents to transmit.
  *
  * @param len The number of bytes to transmit.
  *
  * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER if the buffer is invalid,
  *         or the number of bytes to transmit is greater than `DEVICE_RADIO_MAX_PACKET_SIZE + DEVICE_RADIO_HEADER_SIZE`.
  */
int NRF52RadioDatagram::send(uint8_t *buffer, int len)
{
    if (buffer == NULL || len < 0 || len > NRF52_RADIO_MAX_PACKET_SIZE + NRF52_RADIO_HEADER_SIZE - 1)
        return DEVICE_INVALID_PARAMETER;

    FrameBuffer buf;

    buf.length = len + NRF52_RADIO_HEADER_SIZE - 1;
    buf.version = 1;
    buf.group = 0;
    buf.protocol = NRF52_RADIO_PROTOCOL_DATAGRAM;
    memcpy(buf.payload, buffer, len);

    return radio.send(&buf);
}

/**
  * Transmits the given string onto the broadcast radio.
  *
  * This is a synchronous call that will wait until the transmission of the packet
  * has completed before returning.
  *
  * @param data The packet contents to transmit.
  *
  * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER if the buffer is invalid,
  *         or the number of bytes to transmit is greater than `DEVICE_RADIO_MAX_PACKET_SIZE + DEVICE_RADIO_HEADER_SIZE`.
  */
 #include "CodalDmesg.h"
int NRF52RadioDatagram::send(ManagedBuffer data)
{
    DMESG("TXING len: %d",data.length());
    // uint8_t *pktptr = (uint8_t*)data.getBytes();
    // for (int i = 0; i < data.length(); i++)
    //     DMESG("[%d]",pktptr[i]);

    return send((uint8_t *)data.getBytes(), data.length());
}

/**
  * Transmits the given string onto the broadcast radio.
  *
  * This is a synchronous call that will wait until the transmission of the packet
  * has completed before returning.
  *
  * @param data The packet contents to transmit.
  *
  * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER if the buffer is invalid,
  *         or the number of bytes to transmit is greater than `DEVICE_RADIO_MAX_PACKET_SIZE + DEVICE_RADIO_HEADER_SIZE`.
  */
int NRF52RadioDatagram::send(ManagedString data)
{
    return send((uint8_t *)data.toCharArray(), data.length());
}

/**
  * Protocol handler callback. This is called when the radio receives a packet marked as a datagram.
  *
  * This function process this packet, and queues it for user reception.
  */
void NRF52RadioDatagram::packetReceived()
{
    FrameBuffer *packet = radio.recv();
    int queueDepth = 0;

    // We add to the tail of the queue to preserve causal ordering.
    packet->next = NULL;

    if (rxQueue == NULL)
    {
        rxQueue = packet;
    }
    else
    {
        FrameBuffer *p = rxQueue;
        while (p->next != NULL)
        {
            p = p->next;
            queueDepth++;
        }

        if (queueDepth >= NRF52_RADIO_MAXIMUM_RX_BUFFERS)
        {
            delete packet;
            return;
        }

        p->next = packet;
    }

    Event(DEVICE_ID_RADIO, NRF52_RADIO_EVT_DATAGRAM);
}
