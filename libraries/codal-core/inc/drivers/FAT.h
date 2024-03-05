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

#ifndef DEVICE_FAT_H
#define DEVICE_FAT_H

#include <stdint.h>

namespace codal
{

#define FAT_RESERVED_SECTORS 1
#define FAT_ROOT_DIR_SECTORS 4
#define FAT_SECTORS_PER_FAT(numBl) ((unsigned)(((numBl)*2 + 511) / 512))

#define FAT_START_FAT0(numBl) FAT_RESERVED_SECTORS
#define FAT_START_FAT1(numBl) (FAT_RESERVED_SECTORS + FAT_SECTORS_PER_FAT(numBl))
#define FAT_START_ROOTDIR(numBl) (FAT_RESERVED_SECTORS + 2 * FAT_SECTORS_PER_FAT(numBl))
#define FAT_START_CLUSTERS(numBl)                                                                  \
    (FAT_RESERVED_SECTORS + 2 * FAT_SECTORS_PER_FAT(numBl) + FAT_ROOT_DIR_SECTORS)

typedef struct
{
    uint8_t JumpInstruction[3];
    uint8_t OEMInfo[8];
    uint16_t SectorSize;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FATCopies;
    uint16_t RootDirectoryEntries;
    uint16_t TotalSectors16;
    uint8_t MediaDescriptor;
    uint16_t SectorsPerFAT;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t TotalSectors32;
    uint8_t PhysicalDriveNum;
    uint8_t Reserved;
    uint8_t ExtendedBootSig;
    uint32_t VolumeSerialNumber;
    char VolumeLabel[11];
    uint8_t FilesystemIdentifier[8];
} __attribute__((packed)) FAT_BootBlock;

typedef struct
{
    char name[8];
    char ext[3];
    uint8_t attrs;
    uint8_t reserved;
    uint8_t createTimeFine;
    uint16_t createTime;
    uint16_t createDate;
    uint16_t lastAccessDate;
    uint16_t highStartCluster;
    uint16_t updateTime;
    uint16_t updateDate;
    uint16_t startCluster;
    uint32_t size;
} __attribute__((packed)) DirEntry;

typedef struct
{
    uint8_t seqno;
    uint16_t name0[5];
    uint8_t attrs;
    uint8_t type;
    uint8_t checksum;
    uint16_t name1[6];
    uint16_t startCluster;
    uint16_t name2[2];
} __attribute__((packed)) VFatEntry;
}

#endif
