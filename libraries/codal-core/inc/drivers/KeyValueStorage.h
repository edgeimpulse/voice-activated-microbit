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

#ifndef KEY_VALUE_STORAGE_H
#define KEY_VALUE_STORAGE_H

#include "CodalConfig.h"
#include "ManagedString.h"
#include "ErrorNo.h"
#include "NVMController.h"

#ifndef DEVICE_KEY_VALUE_STORE_OFFSET
#define DEVICE_KEY_VALUE_STORE_OFFSET             -4
#endif

#define KEY_VALUE_STORAGE_MAGIC                   0xC0DA1

#define KEY_VALUE_STORAGE_BLOCK_SIZE              48
#define KEY_VALUE_STORAGE_KEY_SIZE                16
#define KEY_VALUE_STORAGE_VALUE_SIZE              KEY_VALUE_STORAGE_BLOCK_SIZE - KEY_VALUE_STORAGE_KEY_SIZE

#define KEY_VALUE_STORAGE_SCRATCH_WORD_SIZE       64

#define KEY_VALUE_STORAGE_MAX_PAIRS               5

namespace codal
{
  struct KeyValuePair
  {
      uint8_t key[KEY_VALUE_STORAGE_KEY_SIZE];
      uint8_t value[KEY_VALUE_STORAGE_VALUE_SIZE];
  };

  struct KeyValueStore
  {
      uint32_t magic;
      uint32_t size;

      KeyValueStore(uint32_t magic, uint32_t size)
      {
          this->magic = magic;
          this->size = size;
      }

      KeyValueStore()
      {
          this->magic = 0;
          this->size = 0;
      }
  };


  /**
    * Class definition for the KeyValueStorage class.
    * This allows reading and writing of small blocks of data to FLASH memory.
    *
    * This class operates as a key value store, it allows the retrieval, addition
    * and deletion of KeyValuePairs.
    *
    * The first 8 bytes are reserved for the KeyValueStore struct which gives core
    * information such as the number of KeyValuePairs in the store, and whether the
    * store has been initialised.
    *
    * After the KeyValueStore struct, KeyValuePairs are arranged contiguously until
    * the end of the block used as persistent storage.
    *
    * |-------8-------|--------48-------|-----|---------48--------|
    * | KeyValueStore | KeyValuePair[0] | ... | KeyValuePair[N-1] |
    * |---------------|-----------------|-----|-------------------|
    */
  class KeyValueStorage
  {
      uint32_t          flashPagePtr;
      NVMController&    controller;
      uint32_t          *scratch;

      public:

      /**
        * Constructor.
        *
        * Creates an instance of KeyValueStorage which acts like a KeyValueStore
        * that allows the retrieval, addition and deletion of KeyValuePairs.
        * 
        * @param controller The non-volatile storage controller to use
        * @param pageNumber The logical page number for this KeyValueStorage. 
        *                   Optionally use negative number to count from end of address space.
        */
      KeyValueStorage(NVMController& controller, int pageNumber = DEVICE_KEY_VALUE_STORE_OFFSET);

      /**
        * Places a given key, and it's corresponding value into flash at the earliest
        * available point.
        *
        * @param key the unique name that should be used as an identifier for the given data.
        *            The key is presumed to be null terminated.
        *
        * @param data a pointer to the beginning of the data to be persisted.
        *
        * @param dataSize the size of the data to be persisted
        *
        * @return KEY_VALUE_OK on success, KEY_VALUE_INVALID_PARAMETER if the key or size is too large,
        *         KEY_VALUE_NO_RESOURCES if the storage page is full
        */
      int put(const char* key, uint8_t* data, int dataSize);


      /**
        * Places a given key, and it's corresponding value into flash at the earliest
        * available point.
        *
        * @param key the unique name that should be used as an identifier for the given data.
        *
        * @param data a pointer to the beginning of the data to be persisted.
        *
        * @param dataSize the size of the data to be persisted
        *
        * @return KEY_VALUE_OK on success, KEY_VALUE_INVALID_PARAMETER if the key or size is too large,
        *         KEY_VALUE_NO_RESOURCES if the storage page is full
        */
      int put(ManagedString key, uint8_t* data, int dataSize);

      /**
        * Retreives a KeyValuePair identified by a given key.
        *
        * @param key the unique name used to identify a KeyValuePair in flash.
        *
        * @return a pointer to a heap allocated KeyValuePair struct, this pointer will be
        *         NULL if the key was not found in storage.
        *
        * @note it is up to the user to free memory after use.
        */
      KeyValuePair* get(const char* key);

      /**
        * Retreives a KeyValuePair identified by a given key.
        *
        * @param key the unique name used to identify a KeyValuePair in flash.
        *
        * @return a pointer to a heap allocated KeyValuePair struct, this pointer will be
        *         NULL if the key was not found in storage.
        *
        * @note it is up to the user to free memory after use.
        */
      KeyValuePair* get(ManagedString key);

      /**
        * Removes a KeyValuePair identified by a given key.
        *
        * @param key the unique name used to identify a KeyValuePair in flash.
        *
        * @return KEY_VALUE_OK on success, or KEY_VALUE_NO_DATA if the given key
        *         was not found in flash.
        */
      int remove(const char* key);

      /**
        * Removes a KeyValuePair identified by a given key.
        *
        * @param key the unique name used to identify a KeyValuePair in flash.
        *
        * @return KEY_VALUE_OK on success, or KEY_VALUE_NO_DATA if the given key
        *         was not found in flash.
        */
      int remove(ManagedString key);

      /**
        * The size of the flash based KeyValueStore.
        *
        * @return the number of entries in the key value store
        */
      int size();

      /**
       * Erase all contents of this KeyValue store
       */
      int wipe();

      private:

      /**
       * Function to lazily instatiate a scratch buffer
       */
      void scratchReset();

      /**
        * Function for populating the scratch page with a KeyValueStore.
        *
        * @param store the KeyValueStore struct to write to the scratch page.
        */
      void scratchKeyValueStore(KeyValueStore store);

      /**
        * Function for populating the scratch page with a KeyValuePair.
        *
        * @param pair the KeyValuePair struct to write to the scratch page.
        *
        * @param flashPointer the pointer in flash where this KeyValuePair resides. This pointer
        * is used to determine the offset into the scratch page, where the KeyValuePair should
        * be written.
        */
      void scratchKeyValuePair(KeyValuePair pair, int scratchOffset);


  };
}

#endif