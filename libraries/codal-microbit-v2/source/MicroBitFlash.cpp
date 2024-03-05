/*
The MIT License (MIT)
Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.
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

#include "MicroBitConfig.h"
#include "MicroBitFlash.h"
#include "MicroBitDevice.h"
#include "ErrorNo.h"                

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh_soc.h"
#endif

using namespace codal;

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#define WORD_ADDR(x) (((uint32_t)x) & 0xFFFFFFFC)

/*
 * The underlying Nordic libraries that support BLE do not compile cleanly with the stringent GCC settings we employ
 * If we're compiling under GCC, then we suppress any warnings generated from this code (but not the rest of the DAL)
 * The ARM cc compiler is more tolerant. We don't test __GNUC__ here to detect GCC as ARMCC also typically sets this
 * as a compatability option, but does not support the options used...
 */
//#if !defined(__arm)
//#pragma GCC diagnostic ignored "-Wunused-function"
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wunused-parameter"
//#endif

extern "C" void btle_set_user_evt_handler(void (*func)(uint32_t));

/*
 * Return to our predefined compiler settings.
 */

//#if !defined(__arm)
//#pragma GCC diagnostic pop
//#endif

static volatile bool flash_op_complete = false;

/*
 * When SoftDevice is present,
 * define an observer to receive system events.
 * Events are delivered here via nrf_sdh and nrf_sdh_soc
 * See NRF_SDH_DISPATCH_MODEL
 */
#ifdef SOFTDEVICE_PRESENT

static void nvmc_event_handler(uint32_t sys_evt, void *)
{
    if (sys_evt == NRF_EVT_FLASH_OPERATION_SUCCESS)
        flash_op_complete = true;
}

NRF_SDH_SOC_OBSERVER( microbitflash_soc_observer, 0, nvmc_event_handler, NULL);

#endif

/**
  * Default Constructor
  */
MicroBitFlash::MicroBitFlash()
{
}

/**
  * Check if an erase is required to write to a region in flash memory.
  * This is determined if, for any byte:
  * ~O & N != 0x00
  * Where O=Original byte, N = New byte.
  *
  * @param source to write from
  * @param flash_address to write to
  * @param len number of uint8_t to check.
  * @return non-zero if erase required, zero otherwise.
  */
int MicroBitFlash::need_erase(uint8_t* source, uint8_t* flash_addr, int len)
{
    // Erase is necessary if for any byte:
    // O & ~N != 0
    // Where O = original, and N = new byte.

    for(;len>0;len--)
    {
        if((~*(flash_addr++) & *(source++)) != 0x00) return 1;
    }
    return 0;
}

/**
  * Erase an entire page
  * @param page_address address of first word of page
  */
void MicroBitFlash::erase_page(uint32_t* pg_addr)
{
#ifdef SOFTDEVICE_PRESENT
    if (ble_running())
    {
        flash_op_complete = false;
        while(1)
        {
            if (sd_flash_page_erase(((uint32_t)pg_addr)/MICROBIT_CODEPAGESIZE) == NRF_SUCCESS)
                break;

            system_timer_wait_ms(10);
        }
        // Wait for SoftDevice to diable the write operation when it completes...
        while(!flash_op_complete);
    }
    else
#endif
    {
        // Turn on flash erase enable and wait until the NVMC is ready:
        NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een);
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) { }

        // Erase page:
        NRF_NVMC->ERASEPAGE = (uint32_t)pg_addr;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) { }

        // Turn off flash erase enable and wait until the NVMC is ready:
        NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) { }
    }
}

/**
  * Write to flash memory, assuming that a write is valid
  * (using need_erase).
  *
  * @param page_address address of memory to write to.
  *     Must be word aligned.
  * @param buffer address to write from, must be word-aligned.
  * @param len number of uint32_t words to write.
  */
void MicroBitFlash::flash_burn(uint32_t* addr, uint32_t* buffer, int size)
{
#ifdef SOFTDEVICE_PRESENT
    if (ble_running())
    {
        // Schedule SoftDevice to write this memory for us, and wait for it to complete.
        // This happens ASYNCHRONOUSLY when SD is enabled (and synchronously if disabled!!)
        flash_op_complete = false;

        while(1)
        {
            if (sd_flash_write(addr, buffer, size) == NRF_SUCCESS)
                break;

            system_timer_wait_ms(10);
        }

        // Wait for SoftDevice to diable the write operation when it completes...
        while(!flash_op_complete);
    }
    else
#endif
    {
        // Turn on flash write enable and wait until the NVMC is ready:
        NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {};

        for(int i=0;i<size;i++)
        {
            *(addr+i) = *(buffer+i);
            while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {};
        }

        // Turn off flash write enable and wait until the NVMC is ready:
        NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {};
    }
}

/**
  * Writes the given number of bytes to the address in flash specified.
  * Neither address nor buffer need be word-aligned.
  * @param address location in flash to write to.
  * @param buffer location in memory to write from.
  * @length number of bytes to burn
  * @param scratch_addr if specified, scratch page to use. Use default
  *                     otherwise.
  * @return non-zero on sucess, zero on error.
  *
  * Example:
  * @code
  * MicroBitFlash flash();
  * uint32_t word = 0x01;
  * flash.flash_write((uint8_t*)0x38000, &word, sizeof(word))
  * @endcode
  */
int MicroBitFlash::flash_write(void* address, void* from_buffer,
                               int length, void* scratch_addr)
{
    // If no scratch_addr has been supplied use the default
    if(scratch_addr == NULL)
        scratch_addr = (uint32_t *)MICROBIT_DEFAULT_SCRATCH_PAGE;


    // Ensure that scratch_addr is aligned on a page boundary.
    if((uint32_t)scratch_addr & (MICROBIT_CODEPAGESIZE - 1))
        return MICROBIT_INVALID_PARAMETER;

    // Locate the hardware FLASH page used by this operation.
    int page = (uint32_t)address / MICROBIT_CODEPAGESIZE;
    uint32_t* pgAddr = (uint32_t*)(page * MICROBIT_CODEPAGESIZE);

    // offset to write from within page.
    int offset = (uint32_t)address % MICROBIT_CODEPAGESIZE;

    uint8_t* writeFrom = (uint8_t*)pgAddr;
    int start = WORD_ADDR(offset);
    int end = WORD_ADDR((offset+length+4));
    int erase = need_erase((uint8_t *)from_buffer, (uint8_t *)address, length);

    // Preserve the data by writing to the scratch page.
    if(erase)
    {
        if (!scratch_addr)
            return MICROBIT_INVALID_PARAMETER;

        this->erase_page((uint32_t *)scratch_addr);

        this->flash_burn((uint32_t*)scratch_addr, pgAddr, MICROBIT_CODEPAGESIZE/4);
        this->erase_page(pgAddr);
        writeFrom = (uint8_t*)scratch_addr;
        start = 0;
        end = MICROBIT_CODEPAGESIZE;
    }

    uint32_t writeWord = 0;

    for(int i=start;i<end;i++)
    {
        int byteOffset = i%4;

        if(i >= offset && i < (offset + length))
        {
            // Write from buffer.
            writeWord |= (((uint8_t *)from_buffer)[i-offset] << ((byteOffset)*8));
        }
        else
        {
            writeWord |= (writeFrom[i] << ((byteOffset)*8));
        }

        if( ((i+1)%4) == 0)
        {
            this->flash_burn(pgAddr + (i/4), &writeWord, 1);
            writeWord = 0;
        }
    }

    return MICROBIT_OK;
}

