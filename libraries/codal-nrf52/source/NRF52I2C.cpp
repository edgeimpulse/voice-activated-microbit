#include "NRF52I2C.h"
#include "codal_target_hal.h"
#include "CodalDmesg.h"
#include "peripheral_alloc.h"

using namespace codal;

#define MAX_I2C_RETRIES 2 // TODO?

/**
 * Constructor.
 */
NRF52I2C::NRF52I2C(NRF52Pin &sda, NRF52Pin &scl, NRF_TWIM_Type *device) : codal::I2C(sda, scl), sda(sda), scl(scl)
{
    minimumBusIdlePeriod = 0;

#ifdef NRF52I2C_BUS_IDLE_PERIOD
    minimumBusIdlePeriod = NRF52I2C_BUS_IDLE_PERIOD;
#endif

    if(device == NULL)
        p_twim = (NRF_TWIM_Type *)allocate_peripheral(PERI_MODE_I2CM);
    else
        p_twim = (NRF_TWIM_Type *)allocate_peripheral((void *)device);
    
    if (!p_twim)
        target_panic(DEVICE_HARDWARE_CONFIGURATION_ERROR);

    // Disable high-side pin drivers on SDA and SCL pins.
    sda.setDriveMode(6);
    scl.setDriveMode(6);

    // Ensure all I2C drivers on the bus are fully reset
    clearBus();

    // put pins in input mode
    sda.getDigitalValue(PullMode::Up);
    scl.getDigitalValue(PullMode::Up);

    target_wait_us(10);

    nrf_twim_pins_set(p_twim, scl.name, sda.name);
    nrf_twim_frequency_set(p_twim, NRF_TWIM_FREQ_100K);
    nrf_twim_enable(p_twim);

    target_wait_us(10);
}

/**
 * Clear I2C bus. Derived from:
 * https://github.com/NordicSemiconductor/Nordic-Thingy52-FW/blob/126120108879d5bf5d202c9d5cab65e4e9041f58/external/sdk13/components/drivers_nrf/twi_master/nrf_drv_twi.c#L203
 */
void NRF52I2C::clearBus() {
    scl.setDigitalValue(1);
    sda.setDigitalValue(1);

    target_wait_us(4);

    for (int i = 0; i < 9; i++)
    {
        if (sda.getDigitalValue(PullMode::Up))
        {
            if (i == 0)
                return;
            else
                break;
        }

        scl.setDigitalValue(0);
        target_wait_us(4);

        scl.setDigitalValue(1);
        target_wait_us(4);
    }
    sda.setDigitalValue(0);
    target_wait_us(4);
    
    sda.setDigitalValue(1);
}


/** Set the frequency of the I2C interface
 *
 * @param frequency The bus frequency in hertz
 */
int NRF52I2C::setFrequency(uint32_t frequency)
{
    auto freq = NRF_TWIM_FREQ_100K;
    if (frequency >= 250000)
        freq = NRF_TWIM_FREQ_250K;
    if (frequency >= 400000)
        freq = NRF_TWIM_FREQ_400K;

    nrf_twim_disable(p_twim);
    nrf_twim_frequency_set(p_twim, freq);

// Apply Nordic silicon errata #219 if so configured.
// https://infocenter.nordicsemi.com/index.jsp?topic=%2Ferrata_nRF52833_Rev1%2FERR%2FnRF52833%2FRev1%2Flatest%2Fanomaly_833_219.html&anchor=anomaly_833_219
#if CONFIG_ENABLED(NRF52I2C_ERRATA_219)
    if (frequency == 400000)
        p_twim->FREQUENCY = 0x06200000UL;
#endif

    nrf_twim_enable(p_twim);

    return DEVICE_OK;
}

int NRF52I2C::waitForStop(int evt)
{
    int res = DEVICE_OK;
    int locked = 0;

    while (!nrf_twim_event_check(p_twim, (nrf_twim_event_t)evt))
    {
        if (nrf_twim_event_check(p_twim, NRF_TWIM_EVENT_ERROR))
        {
            auto err = p_twim->ERRORSRC;
            p_twim->ERRORSRC = err;

            nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_ERROR);
            nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_RESUME);
            nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_STOP);
            res = DEVICE_I2C_ERROR;

            // Wait for STOP task to complete.
            // Ensures the I2C hardware is idle when the user code initiates a subsequent I2C transcaction.
            while(!nrf_twim_event_check(p_twim, NRF_TWIM_EVENT_STOPPED));
            break;
        }

        // If we started a zero length transmission task (typically a bus probe), then timeout
        // after 1ms. The hardware has no way to relay succesful completion, but may return
        // an error condition.
        if (p_twim->EVENTS_TXSTARTED && p_twim->TXD.MAXCNT == 0 && locked++ == 100)
        {
            res = DEVICE_OK;
            break;
        }

        target_wait_us(10);
    }

    if (minimumBusIdlePeriod)
        target_wait_us(minimumBusIdlePeriod);

    return res;
}

/**
 * Issues a standard, I2C command write to the I2C bus.
 * This consists of:
 *  - Asserting a Start condition on the bus
 *  - Selecting the Slave address (as an 8 bit address)
 *  - Writing a number of raw data bytes provided
 *  - Asserting a Stop condition on the bus
 *
 * The CPU will busy wait until the transmission is complete.
 *
 * @param address The 8bit I2C address of the device to write to
 * @param data pointer to the bytes to write
 * @param len the number of bytes to write
 * @param repeated Suppresses the generation of a STOP condition if set. Default: false;
 *
 * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the write request failed.
 */
int NRF52I2C::write(uint16_t address, uint8_t *data, int len, bool repeated)
{
    address = address >> 1;

    nrf_twim_address_set(p_twim, address);

    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_STOPPED);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_ERROR);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_LASTTX);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_LASTRX);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_TXSTARTED);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_RXSTARTED);

    nrf_twim_tx_buffer_set(p_twim, data, len);

    if (repeated)
        nrf_twim_shorts_set(p_twim, NRF_TWIM_SHORT_LASTTX_SUSPEND_MASK);
    else
        nrf_twim_shorts_set(p_twim, NRF_TWIM_SHORT_LASTTX_STOP_MASK);

    nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_STARTTX);

    if (p_twim->EVENTS_SUSPENDED)
    {
        nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_RESUME);
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_SUSPENDED);
    }

    return waitForStop(repeated ? NRF_TWIM_EVENT_SUSPENDED : NRF_TWIM_EVENT_STOPPED);
}

/**
 * Issues a standard, 2 byte I2C command read to the I2C bus.
 * This consists of:
 *  - Asserting a Start condition on the bus
 *  - Selecting the Slave address (as an 8 bit address)
 *  - reading "len" bytes of raw 8 bit data into the buffer provided
 *  - Asserting a Stop condition on the bus
 *
 * The CPU will busy wait until the transmission is complete.
 *
 * @param address The 8bit I2C address of the device to read from
 * @param data pointer to store the the bytes read
 * @param len the number of bytes to read into the buffer
 * @param repeated Suppresses the generation of a STOP condition if set. Default: false;
 *
 * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the the read request failed.
 */
int NRF52I2C::read(uint16_t address, uint8_t *data, int len, bool repeated)
{
    address = address >> 1;

    nrf_twim_address_set(p_twim, address);

    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_STOPPED);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_ERROR);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_LASTTX);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_LASTRX);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_TXSTARTED);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_RXSTARTED);

    nrf_twim_rx_buffer_set(p_twim, data, len);

    if (!repeated)
        nrf_twim_shorts_set(p_twim, NRF_TWIM_SHORT_LASTRX_STOP_MASK);

    nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_STARTRX);

    if (p_twim->EVENTS_SUSPENDED)
    {
        nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_RESUME);
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_SUSPENDED);
    }

    if (!repeated)
    {
        return waitForStop(NRF_TWIM_EVENT_STOPPED);
    }
    else
    {
        int r = waitForStop(NRF_TWIM_EVENT_LASTRX);
        if (r != DEVICE_OK)
            return r;
        nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_SUSPEND);
        return waitForStop(NRF_TWIM_EVENT_SUSPENDED);
    }
}

/**
 * Performs a typical register read operation to the I2C slave device provided.
 * This consists of:
 *  - Asserting a Start condition on the bus
 *  - Selecting the Slave address (as an 8 bit address, I2C WRITE)
 *  - Selecting a RAM register address in the slave
 *  - Asserting a Stop condition on the bus
 *  - Asserting a Start condition on the bus
 *  - Selecting the Slave address (as an 8 bit address, I2C READ)
 *  - Performing an 8 bit read operation (of the requested register)
 *  - Asserting a Stop condition on the bus
 *
 * The CPU will busy wait until the transmission is complete..
 *
 * @param address 8bit I2C address of the device to read from
 * @param reg The 8bit register address of the to read.
 * @param data A pointer to a memory location to store the result of the read operation
 * @param length The number of mytes to read
 * @param repeated Use a repeated START/START/STOP transaction if true, or independent
 * START/STOP/START/STOP transactions if fasle. Default: true
 *
 * @return DEVICE_OK or DEVICE_I2C_ERROR if the the read request failed.
 */
int NRF52I2C::readRegister(uint16_t address, uint8_t reg, uint8_t *data, int length, bool repeated)
{
    // write followed by a read...
    int ret = write(address, &reg, 1, repeated);

    if (ret)
        return ret;

    ret = read(address, data, length, false);
    return ret;
}

/**
 * Define the minimum bus idle period for this I2C bus.
 * Thise controls the period of time the bus will remain idle between I2C transactions,
 * and also between subsequent write/read operations within a repeated START condition.
 *
 * @param period The minimum bus idle period, in microseconds
 * @return DEVICE_OK on success, or DEVICE_INVALID_PARAMETER
 */
int NRF52I2C::setBusIdlePeriod(int period)
{
    if(period < 0)
        return DEVICE_INVALID_PARAMETER;

    minimumBusIdlePeriod = period;
    return DEVICE_OK;
}
