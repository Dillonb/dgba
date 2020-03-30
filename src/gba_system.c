#include "mem/gbabus.h"
#include "common/log.h"

int cycles = 0;

void gba_system_step(arm7tdmi_t* cpu, gba_ppu_t* ppu, gbabus_t* bus) {
    int dma_cycles = gba_dma();

    if (dma_cycles > 0) {
        cycles += dma_cycles;
    } else {
        if (cpu->halt && (bus->interrupt_enable.raw & bus->IF.raw) == 0) {
            cycles += 1;
        } else {
            cycles += arm7tdmi_step(cpu);
        }
    }

    while (cycles > 4) {
        ppu_step(ppu);
        cycles -= 4;
    }
}