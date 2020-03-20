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

void ppu_step(gba_ppu_t* ppu) {
    ppu->x++;
    if (ppu->x >= GBA_SCREEN_X + GBA_SCREEN_HBLANK) {
        ppu->x = 0;
        ppu->y++;
        if (ppu->y > GBA_SCREEN_Y + GBA_SCREEN_VBLANK) {
            ppu->y = 0;
        }
    }
}
