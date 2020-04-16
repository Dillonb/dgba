#ifndef GBA_DMA_H
#define GBA_DMA_H
#include "gbabus.h"
#include "../audio/audio.h"

void dma_start_trigger(dma_start_time_t trigger);
int dma(int n, DMACNTH_t* cnth, DMAINT_t* dmaint, word sad, word dad, word wc, word max_wc);
void dma_done_hook();

#endif //GBA_DMA_H
