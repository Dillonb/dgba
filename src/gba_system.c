#include "mem/gbabus.h"
#include "mem/gbarom.h"
#include "mem/gbabios.h"
#include "common/log.h"
#include "audio/audio.h"

int cycles = 0;

arm7tdmi_t* cpu = NULL;
gba_ppu_t* ppu = NULL;
gbabus_t* bus = NULL;
gbamem_t* mem = NULL;
gba_apu_t* apu = NULL;
bool should_quit = false;

#define VISIBLE_CYCLES 960
#define HBLANK_CYCLES 272
#define VISIBLE_LINES 160
#define VBLANK_LINES 68

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
    apu = init_apu();
}


int timer_freq[4] = {1, 64, 256, 1024};

INLINE void timer_tick(int cyc) {
    bool previous_overflow = false;
    for (int n = 0; n < 4; n++) {
        bool overflow = false;

        if (bus->TMCNT_H[n].start) {
            //unimplemented(bus->TMCNT_H[n].cascade, "Timer cascade")
            if (bus->TMCNT_H[n].cascade && !previous_overflow) {
                continue;
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

            if (bus->TMCNT_H[n].cascade) {
                bus->TMINT[n].ticks += 1;
            } else {
                bus->TMINT[n].ticks += cyc;
            }

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

                    if (n == 0 || n == 1) {
                        sound_timer_overflow(apu, n);
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

INLINE int inline_gba_cpu_step() {
    cpu_stepped = false;
    int dma_cycles = gba_dma();
    cpu->irq = (bus->interrupt_enable.raw & bus->IF.raw) != 0;
    if (dma_cycles > 0) {
        return dma_cycles;
    } else {
        if (cpu->halt && !cpu->irq) {
            return 1;
        } else {
            cpu_stepped = true;
            return arm7tdmi_step(cpu);
        }
    }
}

INLINE int run_system(int for_cycles) {
    while (for_cycles > 0) {
        int ran = inline_gba_cpu_step();
        timer_tick(ran);
        for (int i = 0; i < ran; i++) {
            apu_tick(apu);
        }
        for_cycles -= ran;
    }
    return for_cycles;
}

INLINE void inline_gba_system_step() {
    cycles += inline_gba_cpu_step();
    while (cycles > 4) {
        timer_tick(1);
        timer_tick(1);
        timer_tick(1);
        timer_tick(1);
        apu_tick(apu);
        apu_tick(apu);
        apu_tick(apu);
        apu_tick(apu);

        cycles -= 4;
    }
}

// Non-inlined version of the above
void gba_system_step() {
    inline_gba_system_step();
}

void gba_system_loop() {
    int extra = 0;
    while (!should_quit) {
        for (int line = 0; line < VISIBLE_LINES; line++) {
            extra = run_system(VISIBLE_CYCLES + extra);
            ppu_hblank(ppu);
            extra = run_system(HBLANK_CYCLES + extra);
            ppu_end_hblank(ppu);
        }
        ppu_vblank(ppu);
        for (int line = 0; line < VBLANK_LINES; line++) {
            extra = run_system(VISIBLE_CYCLES + extra);
            ppu_hblank(ppu);
            extra = run_system(HBLANK_CYCLES + extra);
            ppu_end_hblank(ppu);
        }
        ppu_end_vblank(ppu);
    }
}