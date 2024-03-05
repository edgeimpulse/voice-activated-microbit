#ifndef SINGLE_WIRE_SERIAL_H
#define SINGLE_WIRE_SERIAL_H

#include "Pin.h"
#include "CodalComponent.h"

#define SWS_EVT_DATA_RECEIVED       1
#define SWS_EVT_DATA_SENT           2
#define SWS_EVT_ERROR               3
#define SWS_EVT_DATA_DROPPED        4

// required for gcc-6 (but not 7!)
#undef putc
#undef getc

namespace codal
{
    enum SingleWireMode
    {
        SingleWireRx = 0,
        SingleWireTx,
        SingleWireDisconnected
    };

    class SingleWireSerial : public CodalComponent
    {
        protected:
        virtual void configureRxInterrupt(int enable) = 0;

        virtual int configureTx(int) = 0;

        virtual int configureRx(int) = 0;

        public:
        Pin& p;
        void (*cb) (uint16_t errCode); // one of SWS_EVT...

        SingleWireSerial(Pin& p, uint16_t id = DEVICE_ID_SINGLE_WIRE_SERIAL) : p(p)
        {
            this->id = id;
            this->cb = NULL;
        }

        /**
         * Sets the timer_pointer member variable.
         *
         * @returns DEVICE_OK on success.
         **/
        virtual int setIRQ(void (*cb) (uint16_t errCode))
        {
            this->cb = cb;
            return DEVICE_OK;
        }

        virtual int putc(char c) = 0;
        virtual int getc() = 0;

        virtual int send(uint8_t* buf, int len) = 0;
        virtual int receive(uint8_t* buf, int len) = 0;

        virtual int setBaud(uint32_t baud) = 0;
        virtual uint32_t getBaud() = 0;

        virtual int getBytesReceived() = 0;
        virtual int getBytesTransmitted() = 0;

        virtual int setMode(SingleWireMode sw)
        {
            if (sw == SingleWireRx)
            {
                configureTx(0);
                configureRx(1);
            }
            else if (sw == SingleWireTx)
            {
                configureRx(0);
                configureTx(1);
            }
            else
            {
                configureTx(0);
                configureRx(0);
            }

            return DEVICE_OK;
        }

        virtual int sendBreak() = 0;
    };
}

#endif