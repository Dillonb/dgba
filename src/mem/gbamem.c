#include <stdlib.h>
#include <string.h>
#include "gbamem.h"

gbamem_t* init_mem() {
    gbamem_t* mem = malloc(sizeof(gbamem_t));
    memset(mem, 0, sizeof(gbamem_t));
    return mem;
}
