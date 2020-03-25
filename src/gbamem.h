#ifndef __GBAMEM_H__
#define __GBAMEM_H__

#include <stddef.h>

#include "common/util.h"

// RAM (inside GBA)
#define EWRAM_SIZE 0x40000
#define IWRAM_SIZE 0x8000

// Backup (on cartridge)
#define SRAM_SIZE 0x8000

typedef struct gbamem {
    byte* rom;
    size_t rom_size;
    byte ewram[EWRAM_SIZE];
    byte iwram[IWRAM_SIZE];
    byte* backup;
} gbamem_t;

gbamem_t* init_mem();

#endif
