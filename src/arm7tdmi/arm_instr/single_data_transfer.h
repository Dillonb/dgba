#ifndef __GBA_SINGLE_DATA_TRANSFER_H__
#define __GBA_SINGLE_DATA_TRANSFER_H__

#include <stdbool.h>
#include "../arm7tdmi.h"
#include "arm_instr.h"

void single_data_transfer(arm7tdmi_t* state, arminstr_t* arminstr);
#endif
