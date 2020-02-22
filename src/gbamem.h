#ifndef __GBAMEM_H__
#define __GBAMEM_H__

#include <stddef.h>

#include "util.h"

typedef struct gbamem_t {
    byte* rom;
    size_t rom_size;
} gbamem;

gbamem* init_mem();

#endif
