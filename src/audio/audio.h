#ifndef GBA_AUDIO_H
#define GBA_AUDIO_H

#include <stdbool.h>

#include "../common/util.h"

#define SOUND_FIFO_SIZE 32

typedef union SOUNDCNT_H {
    struct {
        unsigned gbsound_volume:2;
        unsigned dmasound_a_volume:1;
        unsigned dmasound_b_volume:1;
        unsigned:4;
        bool dmasound_a_enable_right:1;
        bool dmasound_a_enable_left:1;
        unsigned dmasound_a_timer_select:1;
        bool dmasound_a_reset_fifo:1;

        bool dmasound_b_enable_right:1;
        bool dmasound_b_enable_left:1;
        unsigned dmasound_b_timer_select:1;
        bool dmasound_b_reset_fifo:1;
    };
    half raw;
} SOUNDCNT_H_t;

typedef union SOUNDCNT_L {
    half raw;
} SOUNDCNT_L_t;

typedef struct sound_fifo {
    byte buf[SOUND_FIFO_SIZE];
    uint64_t read_index;
    uint64_t write_index;
} sound_fifo_t;

typedef struct gba_apu {
    sound_fifo_t fifo[2];

    SOUNDCNT_H_t SOUNDCNT_H;
    SOUNDCNT_L_t SOUNDCNT_L;
} gba_apu_t;

gba_apu_t* init_apu();
void sound_timer_overflow(gba_apu_t* apu, int n);
void write_fifo(gba_apu_t* apu, int n, word value);

#endif //GBA_AUDIO_H
