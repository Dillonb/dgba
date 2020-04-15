#include <stdlib.h>
#include "audio.h"

gba_apu_t* init_apu() {
    gba_apu_t* apu = malloc(sizeof(gba_apu_t));

    return apu;
}
