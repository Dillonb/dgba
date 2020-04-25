#ifndef GBA_DMA_H
#define GBA_DMA_H
#include "gbabus.h"
#include "../audio/audio.h"

void dma_start_trigger(dma_start_time_t trigger);
int gba_dma();
void dma_done_hook();

#endif //GBA_DMA_H
