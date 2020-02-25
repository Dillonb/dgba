#ifndef __GBA_BRANCH_H__
#define __GBA_BRANCH_H__

#include <stdbool.h>
#include "arm7tdmi.h"
void branch(arm7tdmi_t* state, word offset, bool link);
#endif
