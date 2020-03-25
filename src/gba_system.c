#include "gba_system.h"

int cycles = 0;

void gba_system_step(arm7tdmi_t* cpu, gba_ppu_t* ppu) {
    cycles += arm7tdmi_step(cpu);
    while (cycles > 4) {
        ppu_step(ppu);
        cycles -= 4;
    }
}