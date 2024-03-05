#include "cmsis.h"
#include "codal_target_hal.h"
#include "CodalDmesg.h"
#include "CodalCompat.h"
#include "Timer.h"

static int8_t irq_disabled;

// codal::Serial is designed by Polling or Interrupt method without considering DMA.
// In particular, printf forcibly disables interrupts using target_disable_irq(),
// which does not match the behavior of UARTE using Event Interrupt.
// For this reason, when used with the Serial::send() function,
// there are several problems. We can solve this problem a bit
// by checking the irq disabled state in the code. (not all exceptions have been handled yet)
// Therefore, this function is necessary. (This function is used in nRF52Serial extern way)
int8_t target_get_irq_disabled()
{
    return irq_disabled;
}

void target_enable_irq()
{
    irq_disabled--;
    if (irq_disabled <= 0) {
        irq_disabled = 0;
        __enable_irq();
    }
}

void target_disable_irq()
{
    // always disable just in case - it's just one instruction
    __disable_irq();
    irq_disabled++;
    // this used to disable here, only if irq_disabled==1 - this was a race
}

void target_wait_for_event()
{
    __WFE();
}

uint64_t target_get_serial()
{
    return ((uint64_t)NRF_FICR->DEVICEID[1] << 32) | NRF_FICR->DEVICEID[0];
}

void target_reset()
{
    NVIC_SystemReset();
}

extern "C" void _start();
extern "C" __attribute__((weak)) void user_init() {}

#define NUM_VTOR_ENTRIES (NVIC_USER_IRQ_OFFSET + 48)

class VtorCopy
{
public:
    uint32_t vtorStorage[NUM_VTOR_ENTRIES];
    VtorCopy()
    {
        if ((uint32_t)vtorStorage & 0xff)
            target_panic(999);
        auto origVtor = (void*)SCB->VTOR;
        memcpy(vtorStorage, origVtor, sizeof(vtorStorage));
        SCB->VTOR = (uint32_t)vtorStorage;
        DMESG("relocate vtor to %x -> %x %x", origVtor, vtorStorage, SCB->VTOR);
    }
};

// this needs to run after BSS sections are cleared (which happens at the beginning of _start())
__attribute__((used, aligned(512))) static VtorCopy vtorCopy;

extern "C" void target_start()
{
    NRF_NVMC->ICACHECNF = NVMC_ICACHECNF_CACHEEN_Enabled;
    // bring ports back to reset state, in case bootloader messed it up
    for (int i = 0; i < 32; ++i) {
        NRF_P0->PIN_CNF[i] = 2;
#ifdef NRF_P1
        NRF_P1->PIN_CNF[i] = 2;
#endif
    }
    user_init();
    _start();
}

/**
 *  Thread Context for an ARM Cortex core.
 *
 * This is probably overkill, but the ARMCC compiler uses a lot register optimisation
 * in its calling conventions, so better safe than sorry!
 */
struct PROCESSOR_TCB
{
    uint32_t R0;
    uint32_t R1;
    uint32_t R2;
    uint32_t R3;
    uint32_t R4;
    uint32_t R5;
    uint32_t R6;
    uint32_t R7;
    uint32_t R8;
    uint32_t R9;
    uint32_t R10;
    uint32_t R11;
    uint32_t R12;
    uint32_t SP;
    uint32_t LR;
    uint32_t stack_base;
};

PROCESSOR_WORD_TYPE fiber_initial_stack_base()
{
    return DEVICE_STACK_BASE;
}

void *tcb_allocate()
{
    return (void *)malloc(sizeof(PROCESSOR_TCB));
}

/**
 * Configures the link register of the given tcb to have the value function.
 *
 * @param tcb The tcb to modify
 * @param function the function the link register should point to.
 */
void tcb_configure_lr(void *tcb, PROCESSOR_WORD_TYPE function)
{
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    tcbPointer->LR = function;
}

/**
 * Configures the link register of the given tcb to have the value function.
 *
 * @param tcb The tcb to modify
 * @param function the function the link register should point to.
 */
void tcb_configure_sp(void *tcb, PROCESSOR_WORD_TYPE sp)
{
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    tcbPointer->SP = sp;
}

void tcb_configure_stack_base(void *tcb, PROCESSOR_WORD_TYPE stack_base)
{
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    tcbPointer->stack_base = stack_base;
}

PROCESSOR_WORD_TYPE tcb_get_stack_base(void *tcb)
{
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    return tcbPointer->stack_base;
}

PROCESSOR_WORD_TYPE get_current_sp()
{
    return __get_MSP();
}

PROCESSOR_WORD_TYPE tcb_get_sp(void *tcb)
{
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    return tcbPointer->SP;
}

void tcb_configure_args(void *tcb, PROCESSOR_WORD_TYPE ep, PROCESSOR_WORD_TYPE cp,
                        PROCESSOR_WORD_TYPE pm)
{
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    tcbPointer->R0 = (uint32_t)ep;
    tcbPointer->R1 = (uint32_t)cp;
    tcbPointer->R2 = (uint32_t)pm;
}

extern PROCESSOR_WORD_TYPE __end__;
PROCESSOR_WORD_TYPE codal_heap_start = (PROCESSOR_WORD_TYPE)(&__end__);