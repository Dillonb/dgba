#ifndef __GBAMEM_H__
#define __GBAMEM_H__

#include <stddef.h>

#include "util.h"

typedef struct gbamem {
    byte* rom;
    size_t rom_size;
} gbamem_t;

gbamem_t* init_mem();

#endif
