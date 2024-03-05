#ifndef NRF_PERIPHERAL_ALLOC_H
#define NRF_PERIPHERAL_ALLOC_H

#include <stdint.h>
#include "nrf.h"

namespace codal
{

typedef void (*PUserCallback)(void *);

enum PeripheralMode : uint32_t
{
    PERI_MODE_I2CM  = 0x00000001,
    PERI_MODE_I2CS  = 0x00000002,
    PERI_MODE_SPIM  = 0x00000004,
    PERI_MODE_SPIS  = 0x00000008,
    PERI_MODE_UARTE = 0x00000010
};

void *allocate_peripheral(PeripheralMode mode);
void *allocate_peripheral(void *device);
void free_alloc_peri(void* device);
IRQn_Type get_alloc_peri_irqn(void *device);
void set_alloc_peri_irq(void *device, PUserCallback fn, void *userdata);

} // namespace codal

#endif
