//
// Created by dillon on 3/8/20.
//

#ifndef GBA_STATUS_TRANSFER_H
#define GBA_STATUS_TRANSFER_H

#include <stdbool.h>

#include "arm7tdmi.h"

void psr_transfer(arm7tdmi_t* state, bool immediate, unsigned int dt_opcode, unsigned int dt_rn, unsigned int dt_rd, unsigned int dt_operand2);

#endif //GBA_STATUS_TRANSFER_H
