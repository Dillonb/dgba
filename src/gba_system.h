#ifndef GBA_GBA_SYSTEM_H
#define GBA_GBA_SYSTEM_H

#include "arm7tdmi/arm7tdmi.h"
#include "graphics/ppu.h"
#include "mem/gbabus.h"

extern bool cpu_stepped;

extern arm7tdmi_t* cpu;
extern gba_ppu_t* ppu;
extern gbabus_t* bus;
extern gbamem_t* mem;
extern bool should_quit;

void init_gbasystem(const char* romfile, const char* bios_file);
void gba_system_step();
void gba_system_loop();
#endif //GBA_GBA_SYSTEM_H
