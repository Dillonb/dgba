#include <stdlib.h>
#include "gbamem.h"

gbamem_t* init_mem() {
    return malloc(sizeof(gbamem_t));
}
