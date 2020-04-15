#ifndef GBA_AUDIO_H
#define GBA_AUDIO_H

#include <stdbool.h>

#include "../common/util.h"

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

typedef struct gba_apu {
    byte sound_a_buf[32];
    byte sound_a_buf_read_index;
    byte sound_a_buf_write_index;
    byte sound_b_buf[32];
    byte sound_b_buf_read_index;
    byte sound_b_buf_write_index;

    SOUNDCNT_H_t SOUNDCNT_H;
    SOUNDCNT_L_t SOUNDCNT_L;
} gba_apu_t;

gba_apu_t* init_apu();

#endif //GBA_AUDIO_H
