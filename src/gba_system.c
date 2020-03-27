#include "gba_system.h"
#include "mem/gbabus.h"

int cycles = 0;

void gba_system_step(arm7tdmi_t* cpu, gba_ppu_t* ppu) {
    int dma_cycles = gba_dma();

    if (dma_cycles > 0) {
        cycles += dma_cycles;
    } else {
        cycles += arm7tdmi_step(cpu);
    }

    while (cycles > 4) {
        ppu_step(ppu);
        cycles -= 4;
    }
}