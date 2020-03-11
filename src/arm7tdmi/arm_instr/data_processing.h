//
// Created by Dillon Beliveau on 2/25/20.
//

#ifndef GBA_DATA_PROCESSING_H
#define GBA_DATA_PROCESSING_H

#include <stdbool.h>
#include "../arm7tdmi.h"
#include "arm_instr.h"

void data_processing(arm7tdmi_t* state, data_processing_t* data_processing);

#endif //GBA_DATA_PROCESSING_H
