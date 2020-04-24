#ifndef GBA_BLOCK_DATA_TRANSFER_H
#define GBA_BLOCK_DATA_TRANSFER_H

#include <stdbool.h>
#include "../arm7tdmi.h"
#include "arm_instr.h"

void block_data_transfer(arm7tdmi_t* state, arminstr_t * arminstr);

#endif //GBA_BLOCK_DATA_TRANSFER_H
