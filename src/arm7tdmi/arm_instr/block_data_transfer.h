#ifndef GBA_BLOCK_DATA_TRANSFER_H
#define GBA_BLOCK_DATA_TRANSFER_H

#include <stdbool.h>
#include "../arm7tdmi.h"
#include "arm_instr.h"

void block_data_transfer(arm7tdmi_t* state, block_data_transfer_t* instr);

#endif //GBA_BLOCK_DATA_TRANSFER_H
