#ifndef GBA_PUSH_POP_REGISTERS_H
#define GBA_PUSH_POP_REGISTERS_H

#include "../arm7tdmi.h"
#include "thumb_instr.h"

void push_pop_registers(arm7tdmi_t* state, push_pop_registers_t* instr);
#endif //GBA_PUSH_POP_REGISTERS_H
