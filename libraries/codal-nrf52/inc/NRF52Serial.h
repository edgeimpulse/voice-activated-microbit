#ifndef NRF52_SERIAL_H
#define NRF52_SERIAL_H

#include "Pin.h"
#include "nrf.h"
#include "CodalComponent.h"
#include "CodalConfig.h"
#include "Serial.h"
#include "hal/nrf_uarte.h"

#ifndef CONFIG_SERIAL_DMA_BUFFER_SIZE
#define CONFIG_SERIAL_DMA_BUFFER_SIZE   32
#endif

namespace codal
{
    class NRF52Serial : public Serial
    {
        volatile bool is_tx_in_progress_;
        volatile int  bytesProcessed;
        uint8_t dmaBuffer[CONFIG_SERIAL_DMA_BUFFER_SIZE];

        NRF_UARTE_Type *p_uarte_;
        static void _irqHandler(void *self);

        /**
         * Ensures all characters have been processed once a DMA buffer is fully received.
         *
         * This function is called upon an ENDRX hardware event, which is raised when a RX DMA buffer
         * has been filled. Normally there is nothing to do, but in the eventuality that an interrupt has been
         * missed (typically due to CPU contention), this function ensures the codal Serial ringbuffer is synchronised
         * and no characters are lost.
         */
        void updateRxBufferAfterENDRX();

        /**
          * Update DMA RX buffer pointer.
          * 
          * UARTE generates an RXSTARTED event once the DMA buffer geometry has been read.
          * This function implements a DMA enabled double buffer within the codal Serial ringbuffer.
        **/
        void updateRxBufferAfterRXSTARTED();

        /**
          * DMA version of Serial::dataReceviced()
          * 
          * In the A function, only the part of getting data through the getc() function
          * and storing it in the ring buffer is removed.
          * Because DMA stores data directly in the buffer, 
          * this function only manages the ring buffer. (With Event function)
        **/
        void dataReceivedDMA();        

        protected:
        virtual int enableInterrupt(SerialInterruptType t) override;
        virtual int disableInterrupt(SerialInterruptType t) override;
        virtual int configurePins(Pin& tx, Pin& rx) override;

        public:

        /**
         * Constructor
         *
         * @param tx the pin instance to use for transmission
         *
         * @param rx the pin instance to use for reception
         *
         **/
        NRF52Serial(Pin& tx, Pin& rx, NRF_UARTE_Type* device = NULL);

        virtual int putc(char) override;
        virtual int getc() override;
        virtual int setBaudrate(uint32_t baudrate) override;

        ~NRF52Serial();
    };
}

#endif