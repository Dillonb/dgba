#ifndef GBA_LONG_BRANCH_LINK_H
#define GBA_LONG_BRANCH_LINK_H

#include "thumb_instr.h"
#include "../arm7tdmi.h"

void long_branch_link(arm7tdmi_t* state, thumbinstr_t* instr);

#endif //GBA_LONG_BRANCH_LINK_H
