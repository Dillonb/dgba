#ifndef GBA_UNCONDITIONAL_BRANCH_H
#define GBA_UNCONDITIONAL_BRANCH_H

#include "../arm7tdmi.h"
#include "thumb_instr.h"

void unconditional_branch(arm7tdmi_t* state, unconditional_branch_t* instr);
#endif //GBA_UNCONDITIONAL_BRANCH_H
