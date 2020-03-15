#ifndef GBA_MULTIPLY_H
#define GBA_MULTIPLY_H

#include "../arm7tdmi.h"
#include "arm_instr.h"

void multiply(arm7tdmi_t* state, multiply_t* instr);
void multiply_long(arm7tdmi_t* state, multiply_long_t* instr);

#endif //GBA_MULTIPLY_H
