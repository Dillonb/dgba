#ifndef GBA_GBA_SYSTEM_H
#define GBA_GBA_SYSTEM_H

#include "arm7tdmi/arm7tdmi.h"
#include "graphics/ppu.h"
#include "mem/gbabus.h"

extern arm7tdmi_t* cpu;
extern gba_ppu_t* ppu;
extern gbabus_t* bus;
extern gbamem_t* mem;

void init_gbasystem(char* romfile, const char* bios_file);
void gba_system_step(arm7tdmi_t* cpu, gba_ppu_t* ppu, gbabus_t* bus);
#endif //GBA_GBA_SYSTEM_H
