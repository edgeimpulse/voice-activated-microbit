#include "codal_target_hal.h"
#include "CodalDmesg.h"
#include "CodalCompat.h"
#include "Timer.h"

__attribute__((weak)) void target_wait(uint32_t milliseconds)
{
    codal::system_timer_wait_ms(milliseconds);
}

__attribute__((weak)) void target_wait_us(uint32_t us)
{
    codal::system_timer_wait_us(us);
}

__attribute__((weak)) int target_seed_random(uint32_t rand)
{
    return codal::seed_random(rand);
}

__attribute__((weak)) int target_random(int max)
{
    return codal::random(max);
}

__attribute__((weak)) void target_panic(int statusCode)
{
    target_disable_irq();

    DMESG("*** CODAL PANIC : [%d]", statusCode);
    while (1)
    {
    }
}

__attribute__((weak)) void target_deepsleep()
{
    // if not implemented, default to WFI
    target_wait_for_event();
}
