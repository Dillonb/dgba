#include <stdbool.h>

#include "timer.h"
#include "gba_system.h"
#include "common/log.h"

int timer_freq[4] = {1, 64, 256, 1024};

bool process_timer(int n, bool previous_overflow) {
    //unimplemented(bus->TMCNT_H[n].cascade, "Timer cascade")
    if (previous_overflow && bus->TMCNT_H[n].cascade) {
        //logfatal("Timer cascade - need to inc this timer here, and ONLY here. (don't inc normally)")
    }

    bool overflow = false;

    if (bus->TMCNT_H[n].start) {
        if (n == 3) {
            //loginfo("TM3 enabled")
        }
        if (!bus->TMINT[n].previously_enabled) {
            bus->TMINT[n].previously_enabled = true;
            bus->TMINT[n].ticks = 0;
            bus->TMINT[n].value = bus->TMCNT_L[n].timer_reload;
        }

        if (bus->TMCNT_H[n].cascade && !previous_overflow) {
            return false; // Don't inc
        }

        if (++bus->TMINT[n].ticks >= timer_freq[bus->TMCNT_H[n].frequency]) {
            bus->TMINT[n].ticks = 0;
            if (bus->TMINT[n].value == 0xFFFF) {
                bus->TMINT[n].value = bus->TMCNT_L[n].timer_reload;
                overflow = true;
                unimplemented(bus->TMCNT_H[n].timer_irq_enable, "IRQ on timer overflow")
                //printf("Timer %d overflow!\n", n);
            } else {
                bus->TMINT[n].value++;
            }
        }
    } else {
        bus->TMINT[n].previously_enabled = false;
    }

    return overflow;
}

void timer_tick(int cycles) {
    for (int c = 0; c < cycles; c++) {
        bool overflow = false;
        for (int n = 0; n < 4; n++) {
            overflow = process_timer(n, overflow);
        }
    }
}
