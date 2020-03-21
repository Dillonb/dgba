#ifndef GBA_ALU_OPERATIONS_H
#define GBA_ALU_OPERATIONS_H

#include "thumb_instr.h"
#include "../arm7tdmi.h"

void alu_operations(arm7tdmi_t* state, alu_operations_t* instr);
#endif //GBA_ALU_OPERATIONS_H
