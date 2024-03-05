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

#ifndef CODAL_TARGET_HAL_H
#define CODAL_TARGET_HAL_H

#include "platform_includes.h"

extern "C"
{
    void target_enable_irq();

    void target_disable_irq();

    void target_reset();

    void target_wait(uint32_t milliseconds);

    void target_wait_us(uint32_t us);

    int target_seed_random(uint32_t rand);

    int target_random(int max);

    uint64_t target_get_serial();

    void target_wait_for_event();
  
    void target_deepsleep();

    void target_panic(int statusCode);

    PROCESSOR_WORD_TYPE fiber_initial_stack_base();
    /**
      * Configures the link register of the given tcb to have the value function.
      *
      * @param tcb The tcb to modify
      * @param function the function the link register should point to.
      */
    void tcb_configure_lr(void* tcb, PROCESSOR_WORD_TYPE function);

    void* tcb_allocate();

    /**
      * Configures the link register of the given tcb to have the value function.
      *
      * @param tcb The tcb to modify
      * @param function the function the link register should point to.
      */
    void tcb_configure_sp(void* tcb, PROCESSOR_WORD_TYPE sp);

    void tcb_configure_stack_base(void* tcb, PROCESSOR_WORD_TYPE stack_base);

    PROCESSOR_WORD_TYPE tcb_get_stack_base(void* tcb);

    PROCESSOR_WORD_TYPE get_current_sp();

    PROCESSOR_WORD_TYPE tcb_get_sp(void* tcb);

    void tcb_configure_args(void* tcb, PROCESSOR_WORD_TYPE ep, PROCESSOR_WORD_TYPE cp, PROCESSOR_WORD_TYPE pm);
}


#endif
