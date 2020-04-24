#include <stdbool.h>

#include "../arm7tdmi.h"
#include "arm_instr.h"

#ifndef GBA_HALFWORD_DATA_TRANSFER_H
#define GBA_HALFWORD_DATA_TRANSFER_H

void halfword_dt_io(arm7tdmi_t* state, arminstr_t* instr);
void halfword_dt_ro(arm7tdmi_t* state, arminstr_t* instr);

#endif //GBA_HALFWORD_DATA_TRANSFER_H
