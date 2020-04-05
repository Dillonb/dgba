#include "mem/gbabus.h"
#include "common/log.h"
#include "mem/gbarom.h"
#include "mem/gbabios.h"

int cycles = 0;

arm7tdmi_t* cpu;
gba_ppu_t* ppu;
gbabus_t* bus;
gbamem_t* mem;

void init_gbasystem(char* romfile, const char* bios_file) {
    mem = init_mem();

    load_gbarom(romfile, mem);
    if (bios_file) {
        load_alternate_bios(bios_file);
    }

    // Initialize the CPU, hook it up to the GBA bus
    cpu = init_arm7tdmi(gba_read_byte,
                                    gba_read_half,
                                    gba_read_word,
                                    gba_write_byte,
                                    gba_write_half,
                                    gba_write_word);
    ppu = init_ppu();
    bus = init_gbabus(mem, cpu, ppu);
}

void gba_system_step() {
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