#include "CodalConfig.h"
#include "ErrorNo.h"
#include "CodalDmesg.h"
#include "peripheral_alloc.h"
#include "codal_target_hal.h"

namespace codal
{

struct Peripheral
{
    void *device;
    IRQn_Type irqn;
    PeripheralMode modes;
};

static const Peripheral peris[] = {
    //
    {NRF_SPIM0, SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn, 
        (PeripheralMode)(PERI_MODE_I2CM | PERI_MODE_I2CS|
                         PERI_MODE_SPIM | PERI_MODE_SPIS)},
    {NRF_SPIM1, SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn,
        (PeripheralMode)(PERI_MODE_I2CM | PERI_MODE_I2CS|
                         PERI_MODE_SPIM | PERI_MODE_SPIS)},
    {NRF_SPIM2, SPIM2_SPIS2_SPI2_IRQn,
        (PeripheralMode)(PERI_MODE_SPIM | PERI_MODE_SPIS)},
    {NRF_UARTE0, UARTE0_UART0_IRQn, (PeripheralMode)PERI_MODE_UARTE},
#if defined(NRF_SPIM3)
    {NRF_SPIM3, SPIM3_IRQn, (PeripheralMode)PERI_MODE_SPIM},
#endif     
#if defined(NRF_UARTE1)
    {NRF_UARTE1, UARTE1_IRQn, (PeripheralMode)PERI_MODE_UARTE},
#endif
};

#define MAX_NUM_ALLOCATABLE_PERI (uint8_t)(sizeof(peris) / sizeof(peris[0]))

static PUserCallback irq_callback[MAX_NUM_ALLOCATABLE_PERI];
static void *irq_callback_data[MAX_NUM_ALLOCATABLE_PERI];
static uint32_t used_peris;

// To be able to select a specific device
void *allocate_peripheral(void* device)
{
    int i = MAX_NUM_ALLOCATABLE_PERI - 1;
    while (i >= 0)
    {
        if (!(used_peris & (1 << i)) && (peris[i].device == device))
        {
            used_peris |= 1 << i;
            return peris[i].device;
        }
        i--;
    }
    return NULL;
}

void *allocate_peripheral(PeripheralMode mode)
{
    int i = MAX_NUM_ALLOCATABLE_PERI - 1;
    while (i >= 0)
    {
        if (!(used_peris & (1 << i)) && (peris[i].modes & mode))
        {
            used_peris |= 1 << i;
            return peris[i].device;
        }
        i--;
    }
    return NULL;
}

void free_alloc_peri(void* device)
{
    int i = MAX_NUM_ALLOCATABLE_PERI - 1;
    while (i >= 0)
    {
        if (peris[i].device == device)
        {
            used_peris &= ~(1 << i);
            break;
        }
        i--;
    }
}

static int get_alloc_peri_idx(void *device)
{
    for (int i = 0; i < MAX_NUM_ALLOCATABLE_PERI; ++i)
        if (peris[i].device == device)
            return i;
    target_panic(DEVICE_HARDWARE_CONFIGURATION_ERROR);
    return -1;
}

IRQn_Type get_alloc_peri_irqn(void *device)
{
    IRQn_Type IRQn = (IRQn_Type)0xFF;
    int idx = get_alloc_peri_idx(device);
    if(idx >= 0)
        IRQn = peris[idx].irqn;

    return IRQn;
}

void set_alloc_peri_irq(void *device, PUserCallback fn, void *userdata)
{
    int i = get_alloc_peri_idx(device);
    irq_callback[i] = fn;
    irq_callback_data[i] = userdata;
}

#define DEF_IRQ(name, id)                           \
    extern "C" void name()                          \
    {                                               \
        irq_callback[id](irq_callback_data[id]);    \
    }

DEF_IRQ(SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQHandler, 0)
DEF_IRQ(SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQHandler, 1)
DEF_IRQ(SPIM2_SPIS2_SPI2_IRQHandler, 2)
DEF_IRQ(UARTE0_UART0_IRQHandler, 3)
#if defined(NRF_SPIM3)
DEF_IRQ(SPIM3_IRQHandler, 4)
#endif
#if defined(NRF_UARTE1)
DEF_IRQ(UARTE1_IRQHandler, 5)
#endif


} // namespace codal
