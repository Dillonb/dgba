#ifndef __GBA_BRANCH_H__
#define __GBA_BRANCH_H__

#include <stdbool.h>
#include "../arm7tdmi.h"
void branch_exchange(arm7tdmi_t* state, byte opcode, byte rn);
void branch(arm7tdmi_t* state, word offset, bool link);
#endif
