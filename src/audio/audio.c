#include <SDL.h>
#include "audio.h"
#include "../common/log.h"
#include "../mem/dma.h"


SDL_AudioSpec audio_spec;
SDL_AudioSpec request;
SDL_AudioDeviceID audio_dev;

#ifdef ENABLE_AUDIO
void audio_callback(void* userdata, Uint8* stream, int length) {
    gba_apu_t* apu = (gba_apu_t*)userdata;
    float* out = (float*)stream;
    for (int i = 0; i < length / sizeof(float); i++) {
        if (apu->bigbuffer.read_index < apu->bigbuffer.write_index) {
            apu->apu_last_sample = apu->bigbuffer.buf[(apu->bigbuffer.read_index++) % AUDIO_BIGBUFFER_SIZE];
        }
        *out++ = apu->apu_last_sample;
    }
}

#define s8_min (-127.0f)
#define s8_max (128.0f)
#define f32_min (-1.0f)
#define f32_max (1.0f)

INLINE float convert_sample(byte sample) {
    int8_t ssample = sample;
    return ((((float)ssample - s8_min) * (f32_max - f32_min)) / (s8_max - s8_min)) + f32_min;
}

void apu_push_sample(gba_apu_t* apu) {
    uint64_t size = apu->bigbuffer.write_index - apu->bigbuffer.read_index;
    if (size < AUDIO_BIGBUFFER_SIZE) {
        float sample = (convert_sample(apu->fifo[0].sample) + convert_sample(apu->fifo[1].sample)) / 2;
        apu->bigbuffer.buf[(apu->bigbuffer.write_index++) % AUDIO_BIGBUFFER_SIZE] = sample;
    }
}

#endif
gba_apu_t* init_apu(bool enable_audio) {
    gba_apu_t* apu = malloc(sizeof(gba_apu_t));
    memset(apu, 0, sizeof(gba_apu_t));
    apu->enable_audio = enable_audio;
#ifdef ENABLE_AUDIO
    if (apu->enable_audio) {
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            logfatal("SDL couldn't initialize! %s", SDL_GetError())
        }

        memset(&request, 0, sizeof(request));

        request.freq = AUDIO_SAMPLE_RATE;
        request.format = AUDIO_F32SYS;
        request.channels = 1;
        request.samples = 1024;
        request.callback = audio_callback;
        request.userdata = apu;
        audio_dev = SDL_OpenAudioDevice(NULL, 0, &request, &audio_spec, 0);
        unimplemented(request.format != audio_spec.format, "Request != got")

        if (audio_dev == 0) {
            logfatal("Failed to initialize SDL audio: %s", SDL_GetError());
        }

        SDL_PauseAudioDevice(audio_dev, false);
    }
#endif
    return apu;
}

void write_fifo(gba_apu_t* apu, int channel, word value) {
#ifdef ENABLE_AUDIO
    if (!apu->enable_audio) {
        return;
    }
    unimplemented(channel > 1, "tried to fill FIFO >1")
    int size = apu->fifo[channel].write_index - apu->fifo[channel].read_index;
    if (size <= 28) {
        for (int b = 0; b < 4; b++) {
            byte sample = (value >> (b * 8)) & 0xFF;
            apu->fifo[channel].buf[(apu->fifo[channel].write_index++) % SOUND_FIFO_SIZE] = sample;
        }
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
    } else {
        apu->fifo[channel].sample = 0;
    }
}
#endif
void sound_timer_overflow(gba_apu_t* apu, int n) {
#ifdef ENABLE_AUDIO
    if (!apu->enable_audio) {
        return;
    }
    unimplemented(n > 1, "DMA sound from timer >1")

    if (apu->SOUNDCNT_H.dmasound_a_timer_select == n) {
        dmasound_tick(apu, 0);
    }

    if (apu->SOUNDCNT_H.dmasound_b_timer_select == n) {
        dmasound_tick(apu, 1);
    }
    gba_dma();
#endif
}
