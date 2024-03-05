#ifndef UF2FORMAT_H
#define UF2FORMAT_H 1

#include <stdint.h>
#include <stdbool.h>

// All entries are little endian.

// if you increase that, you will also need to update the linker script file
#define APP_START_ADDRESS 0x00002000

#define UF2_MAGIC_START0 0x0A324655UL // "UF2\n"
#define UF2_MAGIC_START1 0x9E5D5157UL // Randomly selected
#define UF2_MAGIC_END 0x0AB16F30UL    // Ditto

// If set, the block is "comment" and should not be flashed to the device
#define UF2_FLAG_NOFLASH 0x00000001

typedef struct
{
    // 32 byte header
    uint32_t magicStart0;
    uint32_t magicStart1;
    uint32_t flags;
    uint32_t targetAddr;
    uint32_t payloadSize;
    uint32_t blockNo;
    uint32_t numBlocks;
    uint32_t reserved;

    // raw data;
    uint8_t data[476];

    // store magic also at the end to limit damage from partial block reads
    uint32_t magicEnd;
} UF2_Block;

typedef struct
{
    uint8_t version;
    uint8_t ep_in;
    uint8_t ep_out;
    uint8_t reserved0;
    uint32_t cbw_tag;
    uint32_t blocks_remaining;
    uint8_t *buffer;
} UF2_HandoverArgs;

typedef void (*UF2_MSC_Handover_Handler)(UF2_HandoverArgs *handover);
typedef void (*UF2_HID_Handover_Handler)(int ep);

// this is required to be exactly 16 bytes long by the linker script
typedef struct
{
    void *reserved0;
    UF2_HID_Handover_Handler handoverHID;
    UF2_MSC_Handover_Handler handoverMSC;
    const char *info_uf2;
} UF2_BInfo;

static inline bool is_uf2_block(void *data)
{
    UF2_Block *bl = (UF2_Block *)data;
    return bl->magicStart0 == UF2_MAGIC_START0 && bl->magicStart1 == UF2_MAGIC_START1 &&
           bl->magicEnd == UF2_MAGIC_END;
}

#ifdef BOOTLOADER_START_ADDR

#define UF2_BINFO ((UF2_BInfo *)(BOOTLOADER_END_ADDR - sizeof(UF2_BInfo)))

#ifndef UF2_INFO_ADDR
#define UF2_INFO_ADDR UF2_BINFO->info_uf2
#endif

static inline bool in_uf2_bootloader_space(const void *addr)
{
    return (BOOTLOADER_START_ADDR + 0xb0) <= (uint32_t)addr && (uint32_t)addr < BOOTLOADER_END_ADDR;
}

static inline const char *uf2_info(void)
{
    if (in_uf2_bootloader_space(UF2_INFO_ADDR))
        return UF2_INFO_ADDR;
    return "N/A";
}

#ifdef UF2_DEFINE_HANDOVER
static inline void hf2_handover(uint8_t ep)
{
    const char *board_info = UF2_INFO_ADDR;
    UF2_HID_Handover_Handler fn = UF2_BINFO->handoverHID;

    if (in_uf2_bootloader_space(board_info) && in_uf2_bootloader_space((const void *)fn) &&
        ((uint32_t)fn & 1))
    {
        // Pass control to bootloader; never returns
        fn(ep & 0xf);
    }
}

static inline void check_uf2_handover(uint8_t *buffer, uint32_t blocks_remaining, uint8_t ep_in,
                                      uint8_t ep_out, uint32_t cbw_tag)
{
    if (!is_uf2_block(buffer))
        return;

    const char *board_info = UF2_BINFO->info_uf2;
    UF2_MSC_Handover_Handler fn = UF2_BINFO->handoverMSC;

    if (in_uf2_bootloader_space(board_info) && in_uf2_bootloader_space((const void *)fn) &&
        ((uint32_t)fn & 1))
    {
        UF2_HandoverArgs hand = {
            1, ep_in, ep_out, 0, cbw_tag, blocks_remaining, buffer,
        };
        // Pass control to bootloader; never returns
        fn(&hand);
    }
}
#endif // UF2_DEFINE_HANDOVER

#endif // BOOTLOADER_START_ADDR

#endif
