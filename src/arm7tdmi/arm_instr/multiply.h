#ifndef GBA_MULTIPLY_H
#define GBA_MULTIPLY_H

#include "../arm7tdmi.h"
#include "arm_instr.h"

void multiply(arm7tdmi_t* state, arminstr_t* instr);
void multiply_long(arm7tdmi_t* state, arminstr_t* arminstr);

#endif //GBA_MULTIPLY_H
