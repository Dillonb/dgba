#include "mem/gbabus.h"
#include "mem/gbarom.h"
#include "mem/gbabios.h"
#include "common/log.h"

int cycles = 0;

arm7tdmi_t* cpu = NULL;
gba_ppu_t* ppu = NULL;
gbabus_t* bus = NULL;
gbamem_t* mem = NULL;
bool should_quit = false;

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


int timer_freq[4] = {1, 64, 256, 1024};

INLINE void timer_tick(int cyc) {
    bool previous_overflow = false;
    for (int n = 0; n < 4; n++) {
        bool overflow = false;

        if (bus->TMCNT_H[n].start) {
            unimplemented(bus->TMCNT_H[n].cascade, "Timer cascade")
            if (previous_overflow && bus->TMCNT_H[n].cascade) {
                logfatal("Timer cascade - need to inc this timer here, and ONLY here. (don't inc normally)")
            }

            if (!bus->TMINT[n].previously_enabled) {
                bus->TMINT[n].previously_enabled = true;
                bus->TMINT[n].ticks = 0;
                bus->TMINT[n].value = bus->TMCNT_L[n].timer_reload;
            }

            if (bus->TMCNT_H[n].cascade && !previous_overflow) {
                previous_overflow = false;
                continue; // Don't inc
            }

            bus->TMINT[n].ticks += cyc;

            while (bus->TMINT[n].ticks >= timer_freq[bus->TMCNT_H[n].frequency]) {
                bus->TMINT[n].ticks -= timer_freq[bus->TMCNT_H[n].frequency];
                if (bus->TMINT[n].value == 0xFFFF) {
                    bus->TMINT[n].value = bus->TMCNT_L[n].timer_reload;
                    overflow = true;
                    if (bus->TMCNT_H[n].timer_irq_enable) {
                        switch (n) {
                            case 0:
                                request_interrupt(IRQ_TIMER0);
                                break;
                            case 1:
                                request_interrupt(IRQ_TIMER1);
                                break;
                            case 2:
                                request_interrupt(IRQ_TIMER2);
                                break;
                            case 3:
                                request_interrupt(IRQ_TIMER3);
                                break;
                        }
                    }
                } else {
                    bus->TMINT[n].value++;
                }
            }
        } else {
            bus->TMINT[n].previously_enabled = false;
        }

        previous_overflow = overflow;
    }
}

bool cpu_stepped = false;


INLINE void _gba_system_step() {
    cpu_stepped = false;
    int dma_cycles = gba_dma();
    cpu->irq = (bus->interrupt_enable.raw & bus->IF.raw) != 0;
    if (dma_cycles > 0) {
        cycles += dma_cycles;
    } else {
        if (cpu->halt && !cpu->irq) {
            cycles += 1;
        } else {
            cpu_stepped = true;
            cycles += arm7tdmi_step(cpu);
        }
    }

    while (cycles > 4) {
        ppu_step(ppu);
        timer_tick(4);
        cycles -= 4;
    }
}

// Non-inlined version of the above
void gba_system_step() {
    _gba_system_step();
}

void gba_system_loop() {
    while (!should_quit) {
        _gba_system_step();
    }
}