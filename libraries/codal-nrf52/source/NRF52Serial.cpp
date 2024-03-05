#include "NRF52Serial.h"
#include "peripheral_alloc.h"
#include "NotifyEvents.h"

using namespace codal;

extern int8_t target_get_irq_disabled();

/**
 * Constructor
 *
 * @param tx the pin instance to use for transmission
 *
 * @param rx the pin instance to use for reception
 *
 **/
NRF52Serial::NRF52Serial(Pin& tx, Pin& rx, NRF_UARTE_Type* device) 
 : Serial(tx, rx), is_tx_in_progress_(false), bytesProcessed(0), p_uarte_(NULL)
{
    if(device != NULL)
        p_uarte_ = (NRF_UARTE_Type*)allocate_peripheral((void*)device);
    else
        p_uarte_ = (NRF_UARTE_Type*)allocate_peripheral(PERI_MODE_UARTE);                    

    if(p_uarte_ == NULL)
        target_panic(DEVICE_HARDWARE_CONFIGURATION_ERROR);

    nrf_uarte_config_t hal_config;
    hal_config.hwfc = NRF_UARTE_HWFC_DISABLED;
    hal_config.parity = NRF_UARTE_PARITY_EXCLUDED;
#if defined(UARTE_CONFIG_STOP_Msk)    
    hal_config.stop = NRF_UARTE_STOP_ONE;
#endif    
#if defined(UARTE_CONFIG_PARITYTYPE_Msk)
    hal_config.paritytype = NRF_UARTE_PARITYTYPE_EVEN;
#endif

    nrf_uarte_baudrate_set(p_uarte_, NRF_UARTE_BAUDRATE_115200);
    nrf_uarte_configure(p_uarte_, &hal_config);
    configurePins(tx, rx);

    nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_RXDRDY);
    nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_ENDRX);
    nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_ENDTX);
    nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_ERROR);
    nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_RXTO);
    nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED);
    nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_RXSTARTED);
    nrf_uarte_shorts_enable(p_uarte_, NRF_UARTE_SHORT_ENDRX_STARTRX);

    nrf_uarte_int_enable(p_uarte_,  NRF_UARTE_INT_RXDRDY_MASK|
                                    NRF_UARTE_INT_RXSTARTED_MASK |
                                    NRF_UARTE_INT_ENDRX_MASK |
                                    NRF_UARTE_INT_ENDTX_MASK |
                                    NRF_UARTE_INT_ERROR_MASK |
                                    NRF_UARTE_INT_RXTO_MASK  |
                                    NRF_UARTE_INT_TXSTOPPED_MASK);

    set_alloc_peri_irq(p_uarte_, &_irqHandler, this);

    IRQn_Type IRQn = get_alloc_peri_irqn(p_uarte_);

    NVIC_SetPriority(IRQn, 1);
    NVIC_ClearPendingIRQ(IRQn);
    NVIC_EnableIRQ(IRQn);            

    nrf_uarte_enable(p_uarte_);
}

NRF52Serial::~NRF52Serial()
{
    nrf_uarte_int_disable(p_uarte_, NRF_UARTE_INT_RXDRDY_MASK|
                                    NRF_UARTE_INT_ENDRX_MASK |
                                    NRF_UARTE_INT_ENDTX_MASK |
                                    NRF_UARTE_INT_ERROR_MASK |
                                    NRF_UARTE_INT_RXTO_MASK  |
                                    NRF_UARTE_INT_TXSTOPPED_MASK);
    NVIC_DisableIRQ(get_alloc_peri_irqn(p_uarte_));

    // Make sure all transfers are finished before UARTE is disabled
    // to achieve the lowest power consumption.
    nrf_uarte_shorts_disable(p_uarte_, NRF_UARTE_SHORT_ENDRX_STARTRX);
    nrf_uarte_task_trigger(p_uarte_, NRF_UARTE_TASK_STOPRX);
    nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED);
    nrf_uarte_task_trigger(p_uarte_, NRF_UARTE_TASK_STOPTX);
    while (!nrf_uarte_event_check(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED))
    {}

    nrf_uarte_disable(p_uarte_);
    nrf_uarte_txrx_pins_disconnect(p_uarte_);

    free_alloc_peri(p_uarte_);
}

void NRF52Serial::_irqHandler(void *self_)
{
    NRF52Serial *self = (NRF52Serial *)self_;
    NRF_UARTE_Type *p_uarte = self->p_uarte_;

    while (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_RXDRDY)){
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_RXDRDY);
        self->dataReceivedDMA();
    }
    
    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ENDRX)){
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ENDRX);
        self->updateRxBufferAfterENDRX();
    }

    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_RXSTARTED)){
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_RXSTARTED);
        self->updateRxBufferAfterRXSTARTED();

    }else if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ERROR)){
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ERROR);
        nrf_uarte_errorsrc_get_and_clear(p_uarte);
    }

    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_RXTO)){
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_RXTO);
        // nrf_uarte_task_trigger(p_uarte, NRF_UARTE_TASK_FLUSHRX);
        uint32_t rx_cnt = nrf_uarte_rx_amount_get(p_uarte);
        while(rx_cnt-- > 0){
            self->dataReceivedDMA();
        }
    }

    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ENDTX)){
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ENDTX);

        self->is_tx_in_progress_ = false;
        if(self->txBufferedSize() > 0){
            self->dataTransmitted();
        }else{
            // Transmitter has to be stopped by triggering STOPTX task to achieve
            // the lowest possible level of the UARTE power consumption.
            nrf_uarte_task_trigger(p_uarte, NRF_UARTE_TASK_STOPTX);
        }
    }

    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_TXSTOPPED)){
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_TXSTOPPED);
        self->is_tx_in_progress_ = false;
    }
}


int NRF52Serial::enableInterrupt(SerialInterruptType t)
{
    if (t == RxInterrupt){
        if(!(status & CODAL_SERIAL_STATUS_RX_BUFF_INIT))
            initialiseRx();

        if(status & CODAL_SERIAL_STATUS_RX_BUFF_INIT){
            nrf_uarte_rx_buffer_set(p_uarte_, dmaBuffer, CONFIG_SERIAL_DMA_BUFFER_SIZE);
            nrf_uarte_int_enable(p_uarte_, NRF_UARTE_INT_ERROR_MASK |
                                            NRF_UARTE_INT_ENDRX_MASK);
            nrf_uarte_task_trigger(p_uarte_, NRF_UARTE_TASK_STARTRX);
        }           
    }else if(t == TxInterrupt){
        if(!is_tx_in_progress_ && txBufferedSize())
        {
            // To prevent the same data from being sent by the TX_DONE event
            // of the UARTE interrupt before processing the ring buffer.
            // Only the order in the Serial.dataTransmitted() function is different.
            uint16_t pre_txBuffTail = txBuffTail;
            txBuffTail = (txBuffTail + 1) % txBuffSize;
            putc((char)txBuff[pre_txBuffTail]);
            if(txBuffTail == txBuffHead){
                Event(DEVICE_ID_NOTIFY, CODAL_SERIAL_EVT_TX_EMPTY);
            }
        }
    }

    return DEVICE_OK;
}

int NRF52Serial::disableInterrupt(SerialInterruptType t)
{
    if (t == RxInterrupt){
        nrf_uarte_int_disable(p_uarte_, NRF_UARTE_INT_ERROR_MASK |
                                        NRF_UARTE_INT_ENDRX_MASK);
    }else if (t == TxInterrupt){
        // IDLE:
        // Since UARTE (DMA) is used, there is no need to turn off and turn off interrupts.
        // In addition, using a function that does not use the codal::Serial structure,
        // such as printf and putc, causes problems, 
        // so it is not right to turn on and off the driver interrupts in this function.
    }

    return DEVICE_OK;
}

int NRF52Serial::setBaudrate(uint32_t baudrate)
{
    nrf_uarte_baudrate_t baud = NRF_UARTE_BAUDRATE_115200;

    switch(baudrate)
    {
        case 9600 : baud = NRF_UARTE_BAUDRATE_9600; break;
        case 38400 : baud = NRF_UARTE_BAUDRATE_38400; break;
        case 57600 : baud = NRF_UARTE_BAUDRATE_57600; break;
        case 115200 : baud = NRF_UARTE_BAUDRATE_115200; break;
        case 230400 : baud = NRF_UARTE_BAUDRATE_230400; break;
        case 921600 : baud = NRF_UARTE_BAUDRATE_921600; break;
        case 1000000 : baud = NRF_UARTE_BAUDRATE_1000000; break;
    }

    nrf_uarte_baudrate_set(p_uarte_, baud);
    return DEVICE_OK;
}

int NRF52Serial::configurePins(Pin& tx, Pin& rx)
{
    this->tx = tx;
    this->rx = rx;
    nrf_uarte_txrx_pins_set(p_uarte_, tx.name, rx.name);
      
    return DEVICE_OK;
}

int NRF52Serial::putc(char c)
{
    int res = DEVICE_OK;

    while(!target_get_irq_disabled() && is_tx_in_progress_);

    if(target_get_irq_disabled()){
        nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_ENDTX);
        nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED);
    }
    is_tx_in_progress_ = true;
    nrf_uarte_tx_buffer_set(p_uarte_, (const uint8_t*)&c, 1);
    nrf_uarte_task_trigger(p_uarte_, NRF_UARTE_TASK_STARTTX);

    // Block for when not using Interrupt. (like codal::Serial::prtinf())
    if(target_get_irq_disabled()){
        bool endtx;
        bool txstopped;
        do
        {
            endtx     = nrf_uarte_event_check(p_uarte_, NRF_UARTE_EVENT_ENDTX);
            txstopped = nrf_uarte_event_check(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED);
        }
        while ((!endtx) && (!txstopped));
        if (txstopped){
            res = DEVICE_INVALID_STATE;
        }else{
            // Transmitter has to be stopped by triggering the STOPTX task to achieve
            // the lowest possible level of the UARTE power consumption.
            nrf_uarte_task_trigger(p_uarte_, NRF_UARTE_TASK_STOPTX);
            while(!nrf_uarte_event_check(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED))
            {}
        }
        is_tx_in_progress_ = false;
    }

    return res;
}

int NRF52Serial::getc()
{
    return this->getChar(codal::SerialMode::ASYNC);
}

void NRF52Serial::dataReceivedDMA()
{
    dataReceived(dmaBuffer[bytesProcessed++]);
}

void NRF52Serial::updateRxBufferAfterENDRX()
{
    int rxBytes = nrf_uarte_rx_amount_get(p_uarte_);
    int rx_cnt = rxBytes - bytesProcessed;

    // If we've dropped any interrupts, recover by processing and missed characters.
    while(rx_cnt--)
        dataReceivedDMA();

    bytesProcessed = 0;
}

void NRF52Serial::updateRxBufferAfterRXSTARTED()
{
    nrf_uarte_rx_buffer_set(p_uarte_, dmaBuffer, CONFIG_SERIAL_DMA_BUFFER_SIZE);
}
