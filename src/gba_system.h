#ifndef GBA_GBA_SYSTEM_H
#define GBA_GBA_SYSTEM_H

#include "arm7tdmi/arm7tdmi.h"
#include "ppu.h"

void gba_system_step(arm7tdmi_t* cpu, gba_ppu_t* ppu);
#endif //GBA_GBA_SYSTEM_H
