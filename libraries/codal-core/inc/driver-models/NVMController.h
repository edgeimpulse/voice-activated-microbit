#ifndef CODAL_NVM_CTRL_H
#define CODAL_NVM_CTRL_H

#include "CodalConfig.h"
#include "ErrorNo.h"

namespace codal
{
    class NVMController
    {
        public:

        NVMController(){};

        /**
         * Determines the logical address of the start of non-volatile memory region
         * 
         * @return The logical address of the first valid logical address in the region of non-volatile memory
         */
        virtual uint32_t getFlashStart()
        {
            return 0;
        }

        /**
         * Determines the logical address of the end of the non-volatile memory region
         * 
         * @return The logical address of the first invalid logical address beyond
         * the non-volatile memory.
         */
        virtual uint32_t getFlashEnd()
        {
            return 0;
        }

        /**
         * Determines the size of a non-volatile memory page. A page is defined as
         * the block of memory the can be efficiently erased without impacted on
         * other regions of memory.
         * 
         * @return The size of a single page in bytes.
         */
        virtual uint32_t getPageSize()
        {
            return 0;
        }

        /**
         * Determines the amount of available storage.
         * 
         * @return the amount of available storage, in bytes.
         */
        virtual uint32_t getFlashSize()
        {
            return 0;
        }

        /**
         * Reads a block of memory from non-volatile memory into RAM
         * 
         * @param dest The address in RAM in which to store the result of the read operation
         * @param source The logical address in non-voltile memory to read from
         * @param size The number 32-bit words to read.
         */ 
        virtual int read(uint32_t* dest, uint32_t source, uint32_t size)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
         * Writes a block of memory to non-volatile memory.
         * 
         * @param dest The logical address in non-voltile memory to write to
         * @param source The address in RAM of the data to write to non-volatile memory
         * @param size The number 32-bit words to write.
         */ 
        virtual int write(uint32_t dest, uint32_t* source, uint32_t size)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

        /**
         * Erases a given page in non-volatile memory.
         * 
         * @param page The address of the page to erase (logical address of the start of the page).
         */
        virtual int erase(uint32_t page)
        {
            return DEVICE_NOT_IMPLEMENTED;
        }

    };
}

#endif