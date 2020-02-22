#include <stdlib.h>
#include "gbamem.h"

gbamem* init_mem() {
    return malloc(sizeof(gbamem));
}
