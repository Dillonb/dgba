#ifndef GBA_GBA_SYSTEM_H
#define GBA_GBA_SYSTEM_H

#include "arm7tdmi/arm7tdmi.h"
#include "graphics/ppu.h"
#include "mem/gbabus.h"

#define NUM_SAVESTATES 10

extern bool cpu_stepped;

extern arm7tdmi_t* cpu;
extern gba_ppu_t* ppu;
extern gbabus_t* bus;
extern gbamem_t* mem;
extern gba_apu_t* apu;
extern bool should_quit;

void init_gbasystem(const char* romfile, const char* bios_file);
void gba_system_step();
void gba_system_loop();

void save_state(const char* path);
void load_state(const char* path);
#endif //GBA_GBA_SYSTEM_H
