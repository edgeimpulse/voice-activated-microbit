#include "ZSingleWireSerial.h"
#include "nrf.h"
#include "core_cm4.h"
#include "CodalDmesg.h"
#include "peripheral_alloc.h"

using namespace codal;

#define TX_CONFIGURED       ((uint16_t)0x02)
#define RX_CONFIGURED       ((uint16_t)0x04)
#define FIRST_BREAK         ((uint16_t)0x08)

void ZSingleWireSerial::irq_handler(void *inst)
{
    ((ZSingleWireSerial*)inst)->irq_handler();
}

void ZSingleWireSerial::irq_handler()
{
    int eventValue = 0;
    if (uart->EVENTS_ENDRX)
    {
        uart->EVENTS_ENDRX = 0;
        configureRxInterrupt(0);
        eventValue = SWS_EVT_DATA_RECEIVED;
    }
    else if (uart->EVENTS_ENDTX)
    {
        uart->EVENTS_ENDTX = 0;
        configureTxInterrupt(0);
        eventValue = SWS_EVT_DATA_SENT;
    }
    else if (uart->EVENTS_ERROR && (uart->INTEN & UARTE_INTENSET_ERROR_Msk))
    {
        uart->EVENTS_ERROR = 0;
        // if we're in reception mode, stop it (error doesn't automatically do so)
        uart->TASKS_STOPRX = 1;
        while (uart->TASKS_STOPRX);
        // clear error src
        uart->ERRORSRC = uart->ERRORSRC;
        // don't wait for ENDRX event, it takes additional 50uS to arrive
        configureRxInterrupt(0);
        eventValue = SWS_EVT_DATA_RECEIVED;
        //eventValue = SWS_EVT_ERROR;
    }

    if (eventValue > 0 && cb)
    {
        cb(eventValue);
    }
}

void ZSingleWireSerial::configureRxInterrupt(int enable)
{
    if (enable)
        uart->INTENSET = (UARTE_INTENSET_ENDRX_Msk | UARTE_INTENSET_ERROR_Msk);
    else
        uart->INTENCLR = (UARTE_INTENCLR_ENDRX_Msk | UARTE_INTENSET_ERROR_Msk);
}

void ZSingleWireSerial::configureTxInterrupt(int enable)
{
    if (enable)
        uart->INTENSET = (UARTE_INTENSET_ENDTX_Msk);
    else
        uart->INTENCLR = (UARTE_INTENCLR_ENDTX_Msk);
}

int ZSingleWireSerial::configureTx(int enable)
{
    if (enable && !(status & TX_CONFIGURED))
    {
        NRF_P0->DIR |= (1 << p.name);
        NRF_P0->PIN_CNF[p.name] =  3 << 2; // this overrides DIR setting above
        uart->PSEL.TXD = p.name;
        uart->EVENTS_ENDTX = 0;
        uart->ENABLE = 8;
        while(!(uart->ENABLE));
        status |= TX_CONFIGURED;
    }
    else if (status & TX_CONFIGURED)
    {
        uart->TASKS_STOPTX = 1;
        while(uart->TASKS_STOPTX);
        uart->ENABLE = 0;
        while((uart->ENABLE));

        uart->PSEL.TXD = 0xFFFFFFFF;
        status &= ~TX_CONFIGURED;
    }

    return DEVICE_OK;
}

int ZSingleWireSerial::configureRx(int enable)
{
    if (enable && !(status & RX_CONFIGURED))
    {
        NRF_P0->DIR &= ~(1 << p.name);
        NRF_P0->PIN_CNF[p.name] =  3 << 2; // this overrides DIR setting above
        uart->PSEL.RXD = p.name;
        uart->EVENTS_ENDRX = 0;
        uart->EVENTS_ERROR = 0;
        uart->ERRORSRC = uart->ERRORSRC;
        uart->ENABLE = 8;
        while(!(uart->ENABLE));
        status |= RX_CONFIGURED;
    }
    else if (enable == 0 && status & RX_CONFIGURED)
    {
        uart->TASKS_STOPRX = 1;
        while(uart->TASKS_STOPRX);
        uart->ENABLE = 0;
        while((uart->ENABLE));
        uart->PSEL.RXD = 0xFFFFFFFF;
        status &= ~RX_CONFIGURED;
    }

    return DEVICE_OK;
}

ZSingleWireSerial::ZSingleWireSerial(Pin& p) : DMASingleWireSerial(p)
{
    uart = (NRF_UARTE_Type*)allocate_peripheral(PERI_MODE_UARTE);
    if (!uart)
        target_panic(DEVICE_HARDWARE_CONFIGURATION_ERROR);

    status = 0;

    uart->CONFIG = 0;

    // these lines are disabled
    uart->PSEL.CTS = 0xFFFFFFFF;
    uart->PSEL.RTS = 0xFFFFFFFF;

    // this will be set to pin name depending on configure TX/RX
    uart->PSEL.TXD = 0xFFFFFFFF;
    uart->PSEL.RXD = 0xFFFFFFFF;

    setBaud(1000000);

    IRQn_Type irqn = get_alloc_peri_irqn(uart);
    NVIC_DisableIRQ(irqn);
    NVIC_SetPriority(irqn, 1);
    set_alloc_peri_irq(uart, irq_handler, this);
    NVIC_EnableIRQ(irqn);

    status |= DEVICE_COMPONENT_RUNNING;
}

int ZSingleWireSerial::putc(char c)
{
    return DEVICE_NOT_IMPLEMENTED;
}
int ZSingleWireSerial::getc()
{
    return DEVICE_NOT_IMPLEMENTED;
}

int ZSingleWireSerial::send(uint8_t* data, int len)
{
    return DEVICE_NOT_IMPLEMENTED;
}

int ZSingleWireSerial::receive(uint8_t* data, int len)
{
    return DEVICE_NOT_IMPLEMENTED;
}

int ZSingleWireSerial::sendDMA(uint8_t* data, int len)
{
    if (!(status & TX_CONFIGURED))
        setMode(SingleWireTx);

    uart->TXD.PTR = (uint32_t)data;
    uart->TXD.MAXCNT = len;

    configureTxInterrupt(1);

    uart->TASKS_STARTTX = 1;

    return DEVICE_OK;
}

int ZSingleWireSerial::receiveDMA(uint8_t* data, int len)
{
    if (!(status & RX_CONFIGURED))
        setMode(SingleWireRx);

    uart->RXD.PTR = (uint32_t)data;
    uart->RXD.MAXCNT = len;

    configureRxInterrupt(1);

    uart->TASKS_STARTRX = 1;

    return DEVICE_OK;
}

int ZSingleWireSerial::abortDMA()
{
    configureTxInterrupt(0);
    configureRxInterrupt(0);

    uart->RXD.MAXCNT = 0;
    uart->TXD.MAXCNT = 0;

    return DEVICE_OK;
}

int ZSingleWireSerial::setBaud(uint32_t baud)
{
    if (baud == 1000000)
        uart->BAUDRATE = 0x10000000;
    else if (baud == 38400)
        uart->BAUDRATE = 0x009D5000;
    else if (baud == 9600)
        uart->BAUDRATE = 0x00275000;
    else
        // 115200
        uart->BAUDRATE = 0x01D7E000;

    return DEVICE_OK;
}

uint32_t ZSingleWireSerial::getBaud()
{
    if (uart->BAUDRATE == 0x10000000)
        return 1000000;

    if (uart->BAUDRATE == 0x009D5000)
        return 38400;

    if (uart->BAUDRATE == 0x00275000)
        return 9600;

    if (uart->BAUDRATE == 0x01D7E000)
        return 115200;

    return 0;
}

int ZSingleWireSerial::getBytesTransmitted()
{
    return DEVICE_NOT_IMPLEMENTED;
}

int ZSingleWireSerial::getBytesReceived()
{
    return DEVICE_NOT_IMPLEMENTED;
}

int ZSingleWireSerial::sendBreak()
{
    return DEVICE_NOT_IMPLEMENTED;
}