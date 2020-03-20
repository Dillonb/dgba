#include "ppu.h"
#include "common/log.h"

void write_dispcnt(gba_ppu_t* state, half value) {
    state->DISPCNT.raw = value;
    logwarn("DISPCNT write!")
}

gba_ppu_t* init_ppu() {
    gba_ppu_t* ppu = malloc(sizeof(gba_ppu_t));

    ppu->DISPCNT.raw = 0;

    ppu->x = 0;
    ppu->y = 0;

    return ppu;
}
