#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include "audio.h"
#include "../common/log.h"
#include "../mem/gbabus.h"

#define AUDIO_SAMPLE_RATE 48000


SDL_AudioSpec audio_spec;
SDL_AudioSpec request;
SDL_AudioDeviceID audio_dev;

void audio_callback(void* userdata, Uint8* stream, int length) {
    gba_apu_t* apu = (gba_apu_t*)userdata;
    float* out = (float*)stream;

    float fifo_a = (float)apu->fifo[0].sample;
    float fifo_b = (float)apu->fifo[1].sample;

    fifo_a = (fifo_a - 128.f) / 128.0f;
    fifo_b = (fifo_b - 128.0f) / 128.0f;

    float average = ((float)fifo_a + (float)fifo_b) / 2;

    average *= 0.9f;
    if (average > 1 || average < -1) {
        printf("%f 0x%02X 0x%02X\n", average, apu->fifo[0].sample, apu->fifo[1].sample);
    }

    //printf("%lu samples of %f\n", length / sizeof(float), average);

    for (int i = 0; i < length / sizeof(float); i++) {
        *out++ = average;
    }
}


gba_apu_t* init_apu() {
    gba_apu_t* apu = malloc(sizeof(gba_apu_t));
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        logfatal("SDL couldn't initialize! %s", SDL_GetError())
    }

    memset(&request, 0, sizeof(request));

    request.freq = AUDIO_SAMPLE_RATE;
    request.format = AUDIO_F32SYS;
    request.channels = 1;
    request.samples = 4;
    request.callback = audio_callback;
    request.userdata = apu;
    audio_dev = SDL_OpenAudioDevice(NULL, 0, &request, &audio_spec, 0);

    if (audio_dev == 0) {
        logfatal("Failed to initialize SDL audio: %s", SDL_GetError());
    }

    SDL_PauseAudioDevice(audio_dev, false);

    return apu;
}

void write_fifo(gba_apu_t* apu, int channel, word value) {
    unimplemented(channel > 1, "tried to fill FIFO >1")
    int size = apu->fifo[channel].write_index - apu->fifo[channel].read_index;
    if (size <= 28) {
        byte samples[4];
        //for (int b = 3; b >= 0; b--) {
        for (int b = 0; b < 4; b++) {
            byte sample = (value >> (b * 8)) & 0xFF;
            samples[b] = sample;
            apu->fifo[channel].buf[(apu->fifo[channel].write_index++) % SOUND_FIFO_SIZE] = sample;
            size = apu->fifo[channel].write_index - apu->fifo[channel].read_index;
            printf("ch: %d WROTE 0x%02X - wi = %lu ri = %lu, size = %d\n", channel, sample, apu->fifo[channel].write_index, apu->fifo[channel].read_index, size);
        }
        //printf("Turned 0x%08X into 0x%02X 0x%02X 0x%02X 0x%02X\n", value, samples[0], samples[1], samples[2], samples[3]);
    } else {
        printf("Buffer OVERRUN, ignoring write of 0x%08X\n", value);
    }
}

INLINE void dmasound_tick(gba_apu_t* apu, int channel) {
    uint64_t* wi = &apu->fifo[channel].write_index;
    uint64_t* ri = &apu->fifo[channel].read_index;

    if (*ri < *wi) {
        byte sample = apu->fifo[channel].buf[(apu->fifo[channel].read_index++) % SOUND_FIFO_SIZE];
        apu->fifo[channel].sample = sample;
        printf("ch %d GOT A SAMPLE 0x%02X ri %lu wi %lu\n", channel, sample, *ri, *wi);
    } else {
        printf("ch %d BUFFER UNDERRUN ri %lu wi %lu\n", channel, *ri, *wi);
        apu->fifo[channel].sample = 0;
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
