#ifndef GBA_AUDIO_H
#define GBA_AUDIO_H
#define ENABLE_AUDIO

#include <stdbool.h>

#include "../common/util.h"

#define SOUND_FIFO_SIZE 32
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_BIGBUFFER_SIZE (4096)
#define CPU_FREQUENCY (16*1024*1024)
#define SAMPLE_EVERY_CYCLES (CPU_FREQUENCY/AUDIO_SAMPLE_RATE)

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
    byte sample;
} sound_fifo_t;

typedef struct sound_bigbuffer {
    float buf[AUDIO_BIGBUFFER_SIZE];
    uint64_t read_index;
    uint64_t write_index;
} sound_bigbuffer_t;

typedef struct gba_apu {
    sound_fifo_t fifo[2];
    sound_bigbuffer_t bigbuffer;

    SOUNDCNT_H_t SOUNDCNT_H;
    SOUNDCNT_L_t SOUNDCNT_L;
    word apu_cycle_counter;
    float apu_last_sample;
    bool enable_audio;
} gba_apu_t;

gba_apu_t* init_apu(bool enable_audio);
void sound_timer_overflow(gba_apu_t* apu, int n);
void write_fifo(gba_apu_t* apu, int channel, word value, word mask);
void apu_push_sample(gba_apu_t* apu);
#ifdef ENABLE_AUDIO
#define apu_tick(apu) do { if (++apu->apu_cycle_counter > SAMPLE_EVERY_CYCLES) { apu->apu_cycle_counter = 0; apu_push_sample(apu); } } while(0)
#else
#define apu_tick(apu) do {} while(0)
#endif
#endif //GBA_AUDIO_H
