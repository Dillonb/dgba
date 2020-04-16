#include <stdlib.h>
#include <stdio.h>
#include "audio.h"
#include "../common/log.h"

gba_apu_t* init_apu() {
    gba_apu_t* apu = malloc(sizeof(gba_apu_t));

    return apu;
}

void write_fifo(gba_apu_t* apu, int n, word value) {
    unimplemented(n > 1, "tried to fill FIFO >1")
    for (int b = 0; b < 4; b++) {
        apu->fifo[n].buf[(apu->fifo[n].write_index++) % SOUND_FIFO_SIZE] = (value >> (b * 8)) & 0xFF;
    }
}

INLINE void dmasound_tick(gba_apu_t* apu, int n) {
    // TODO: send this sample to the sound processor instead
    if (apu->fifo[n].read_index < apu->fifo[n].write_index) {
        byte sample = apu->fifo[n].buf[(apu->fifo[n].read_index++) % SOUND_FIFO_SIZE];
        printf("Sample for DMA sound %s: 0x%02X\n", n == 0 ? "A" : "B", sample);
    }
}

void sound_timer_overflow(gba_apu_t* apu, int n) {
    unimplemented(n > 1, "DMA sound from timer >1")

    if (apu->SOUNDCNT_H.dmasound_a_timer_select == n) {
        dmasound_tick(apu, 0);
    }

    if (apu->SOUNDCNT_H.dmasound_b_timer_select == n) {
        dmasound_tick(apu, 1);
    }
}
