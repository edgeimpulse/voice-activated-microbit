#ifndef CODAL_PEARSON_HASH_H
#define CODAL_PEARSON_HASH_H

#include "CodalConfig.h"
#include "ManagedString.h"

namespace codal
{
    /**
      * PearsonHashing is a simple way to turn a string into a number.
      * Unfortunately, uniqueness guarantees are not too high, and scales with the bit depth.
      **/
    class PearsonHash
    {
        static uint32_t hashN(ManagedString s, uint8_t byteCount);

        public:
        static uint8_t hash8(ManagedString s);
        static uint16_t hash16(ManagedString s);
        static uint32_t hash32(ManagedString s);
    };
}

#endif