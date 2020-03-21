#ifndef GBA_CONDITIONAL_BRANCH_H
#define GBA_CONDITIONAL_BRANCH_H

#include "thumb_instr.h"
#include "../arm7tdmi.h"

void conditional_branch(arm7tdmi_t* state, conditional_branch_t* instr);
#endif //GBA_CONDITIONAL_BRANCH_H
