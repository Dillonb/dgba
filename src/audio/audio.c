#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include "audio.h"
#include "../common/log.h"
#include "../mem/gbabus.h"



SDL_AudioSpec audio_spec;
SDL_AudioSpec request;
SDL_AudioDeviceID audio_dev;
uint32_t apu_cycle_counter = 0;

#ifdef ENABLE_AUDIO
int underruns = 0;
float apu_last_sample = 0.0f;
void audio_callback(void* userdata, Uint8* stream, int length) {
    gba_apu_t* apu = (gba_apu_t*)userdata;
    float* out = (float*)stream;
    for (int i = 0; i < length / sizeof(float); i++) {
        if (apu->bigbuffer.read_index < apu->bigbuffer.write_index) {
            apu_last_sample = apu->bigbuffer.buf[(apu->bigbuffer.read_index++) % AUDIO_BIGBUFFER_SIZE];
        } else {
            //printf("UNDERRUN %d!\n", ++underruns);
        }
        *out++ = apu_last_sample;
    }
}
byte apu_last_byte_sample = 0x00;
void apu_push_sample(gba_apu_t* apu) {
    uint64_t size = apu->bigbuffer.write_index - apu->bigbuffer.read_index;
    if (size < AUDIO_BIGBUFFER_SIZE) {
        //float fifo_a = (float)apu->fifo[0].sample;
        //fifo_a = (fifo_a - 127.0f) / 127.0f;
        byte sample = apu->fifo[0].sample;
        if (sample != apu_last_byte_sample) {
            printf("Playing 0x%02X\n", sample);
            apu_last_byte_sample = sample;
        }

        float x = (((int)(sample >> 1)) | (sample & 0x40) << 24)/128.0f;
        x *= 0.01f;
        //float fifo_b = (float)apu->fifo[1].sample;

        //fifo_a *= 0.4f;
        //fifo_b = (fifo_b - 128.0f) / 128.0f;

        //float average = ((float)fifo_a + (float)fifo_b) / 2;

        //average *= 0.9f;
        apu->bigbuffer.buf[(apu->bigbuffer.write_index++) % AUDIO_BIGBUFFER_SIZE] = x;
    } else {
        logwarn("Buffer overrun! Skipping a sample")
    }
}

#endif
gba_apu_t* init_apu() {
    gba_apu_t* apu = malloc(sizeof(gba_apu_t));
#ifdef ENABLE_AUDIO
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        logfatal("SDL couldn't initialize! %s", SDL_GetError())
    }

    memset(&request, 0, sizeof(request));

    request.freq = AUDIO_SAMPLE_RATE;
    request.format = AUDIO_F32SYS;
    request.channels = 1;
    request.samples = 32;
    request.callback = audio_callback;
    request.userdata = apu;
    audio_dev = SDL_OpenAudioDevice(NULL, 0, &request, &audio_spec, 0);
    unimplemented(request.format != audio_spec.format, "Request != got")

    if (audio_dev == 0) {
        logfatal("Failed to initialize SDL audio: %s", SDL_GetError());
    }

    SDL_PauseAudioDevice(audio_dev, false);
#endif
    return apu;
}

//#define REVERSE

void write_fifo(gba_apu_t* apu, int channel, word value) {
#ifdef ENABLE_AUDIO
    unimplemented(channel > 1, "tried to fill FIFO >1")
    int size = apu->fifo[channel].write_index - apu->fifo[channel].read_index;
    if (size <= 28) {
        //byte samples[4];
#ifdef REVERSE
        for (int b = 3; b >= 0; b--) {
#else
        for (int b = 0; b < 4; b++) {
#endif
            byte sample = (value >> (b * 8)) & 0xFF;
            //samples[b] = sample;
            apu->fifo[channel].buf[(apu->fifo[channel].write_index++) % SOUND_FIFO_SIZE] = sample;
            //size = apu->fifo[channel].write_index - apu->fifo[channel].read_index;
            printf("ch: %d WROTE 0x%02X - wi = %lu ri = %lu, size = %d\n", channel, sample, apu->fifo[channel].write_index, apu->fifo[channel].read_index, size);
        }
        //printf("Turned 0x%08X into 0x%02X 0x%02X 0x%02X 0x%02X\n", value, samples[0], samples[1], samples[2], samples[3]);
    } else {
        printf("Buffer OVERRUN, ignoring write of 0x%08X\n", value);
    }
#endif
}

#ifdef ENABLE_AUDIO
INLINE void dmasound_tick(gba_apu_t* apu, int channel) {
    uint64_t* wi = &apu->fifo[channel].write_index;
    uint64_t* ri = &apu->fifo[channel].read_index;

    if (*ri < *wi) {
        byte sample = apu->fifo[channel].buf[(apu->fifo[channel].read_index++) % SOUND_FIFO_SIZE];
        apu->fifo[channel].sample = sample;
        //printf("ch %d GOT A SAMPLE 0x%02X ri %lu wi %lu\n", channel, sample, *ri, *wi);
    } else {
        //printf("ch %d BUFFER UNDERRUN ri %lu wi %lu\n", channel, *ri, *wi);
        apu->fifo[channel].sample = 0;
    }
}
#endif
void sound_timer_overflow(gba_apu_t* apu, int n) {
#ifdef ENABLE_AUDIO
    unimplemented(n > 1, "DMA sound from timer >1")

    if (apu->SOUNDCNT_H.dmasound_a_timer_select == n) {
        dmasound_tick(apu, 0);
    }

    if (apu->SOUNDCNT_H.dmasound_b_timer_select == n) {
        dmasound_tick(apu, 1);
    }
#endif
}
