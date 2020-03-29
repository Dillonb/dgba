//
// Created by dillon on 3/29/20.
//

#ifndef GBA_DMA_H
#define GBA_DMA_H
#include "gbabus.h"

int dma(int n, DMACNTH_t* cnth, DMAINT_t* dmaint, word sad, word dad, word wc, word max_wc);

#endif //GBA_DMA_H
