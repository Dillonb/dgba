#ifndef GBA_STATUS_TRANSFER_H
#define GBA_STATUS_TRANSFER_H

#include <stdbool.h>

#include "../arm7tdmi.h"
#include "arm_instr.h"

void psr_transfer(arm7tdmi_t* state, arminstr_t* arminstr);

#endif //GBA_STATUS_TRANSFER_H
