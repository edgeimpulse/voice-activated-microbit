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

#ifndef CODAL_MBED_SPI_H
#define CODAL_MBED_SPI_H

#include "CodalConfig.h"
#include "hal/nrf_spi.h"
#include "hal/nrf_spim.h"
#include "codal-core/inc/driver-models/SPI.h"
#include "codal-core/inc/driver-models/Pin.h"

namespace codal
{

/**
 * Class definition for SPI service, derived from ARM mbed.
 */
class NRF52SPI : public codal::SPI
{
    codal::Pin &mosi;
    codal::Pin &miso;
    codal::Pin &sck;
    nrf_spim_frequency_t freq;
    IRQn_Type IRQn;
    uint8_t mode;
    uint8_t configured;
    uint8_t sendCh;
    uint8_t recvCh;
    NRF_SPIM_Type *p_spim;

    PVoidCallback doneHandler;
    void *doneHandlerArg;

    void config();

    int xfer(uint8_t const *p_tx_buffer, uint32_t tx_length, uint8_t *p_rx_buffer,
             uint32_t rx_length, PVoidCallback doneHandler, void *arg);

    static void _irqDoneHandler(void *self);
public:

    /**
     * Initialize SPI instance with given pins.
     *
     * Default setup is 1 MHz, 8 bit, mode 0.
     */
    NRF52SPI(codal::Pin &mosi, codal::Pin &miso, codal::Pin &sclk, NRF_SPIM_Type* device = NULL);

    /** Set the frequency of the SPI interface
     *
     * @param frequency The bus frequency in hertz
     */
    virtual int setFrequency(uint32_t frequency);

    int frequency(uint32_t frequency) { return setFrequency(frequency); }

    /** Set the mode of the SPI interface
     *
     * @param mode Clock polarity and phase mode (0 - 3)
     * @param bits Number of bits per SPI frame (4 - 16)
     *
     * @code
     * mode | POL PHA
     * -----+--------
     *   0  |  0   0
     *   1  |  0   1
     *   2  |  1   0
     *   3  |  1   1
     * @endcode
     */
    virtual int setMode(int mode, int bits = 8);

    int format(int bits, int mode) { return setMode(mode, bits); }

    /**
     * Writes the given byte to the SPI bus.
     *
     * The CPU will busy wait until the transmission is complete.
     *
     * @param data The data to write.
     * @return Response from the SPI slave or DEVICE_SPI_ERROR if the the write request failed.
     */
    virtual int write(int data);

    /**
     * Writes and reads from the SPI bus concurrently. Waits (possibly un-scheduled) for transfer to finish.
     *
     * Either buffer can be NULL.
     */
    virtual int transfer(const uint8_t *txBuffer, uint32_t txSize, uint8_t *rxBuffer,
                         uint32_t rxSize);

    /**
     * Writes and reads from the SPI bus concurrently. Finally, calls doneHandler (possibly in IRQ context).
     *
     * Either buffer can be NULL.
     */
    virtual int startTransfer(const uint8_t *txBuffer, uint32_t txSize, uint8_t *rxBuffer,
                         uint32_t rxSize, PVoidCallback doneHandler, void *arg);
};
}

#endif
