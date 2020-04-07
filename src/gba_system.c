#include "mem/gbabus.h"
#include "mem/gbarom.h"
#include "mem/gbabios.h"
#include "timer.h"

int cycles = 0;

arm7tdmi_t* cpu = NULL;
gba_ppu_t* ppu = NULL;
gbabus_t* bus = NULL;
gbamem_t* mem = NULL;

void init_gbasystem(const char* romfile, const char* bios_file) {
    mem = init_mem();

    load_gbarom(romfile);
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
    bus = init_gbabus();
}

void gba_system_step() {
    int dma_cycles = gba_dma();
    cpu->irq = (bus->interrupt_enable.raw & bus->IF.raw) != 0;
    if (dma_cycles > 0) {
        cycles += dma_cycles;
    } else {
        if (cpu->halt && !cpu->irq) {
            cycles += 1;
        } else {
            cycles += arm7tdmi_step(cpu);
        }
    }

    timer_tick(cycles);

    while (cycles > 4) {
        ppu_step(ppu);
        cycles -= 4;
    }
}