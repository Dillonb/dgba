#ifndef __GBA_BRANCH_H__
#define __GBA_BRANCH_H__

#include <stdbool.h>
#include "../arm7tdmi.h"
#include "arm_instr.h"

void branch_exchange(arm7tdmi_t* state, branch_exchange_t* instr);
void branch(arm7tdmi_t* state, branch_t* instr);
#endif
