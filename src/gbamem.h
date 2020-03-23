#ifndef __GBAMEM_H__
#define __GBAMEM_H__

#include <stddef.h>

#include "common/util.h"

#define EWRAM_SIZE 0x40000
#define IWRAM_SIZE 0x8000

typedef struct gbamem {
    byte* rom;
    size_t rom_size;
    byte ewram[EWRAM_SIZE];
    byte iwram[IWRAM_SIZE];
} gbamem_t;

gbamem_t* init_mem();

#endif
