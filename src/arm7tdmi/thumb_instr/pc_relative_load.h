#ifndef GBA_PC_RELATIVE_LOAD_H
#define GBA_PC_RELATIVE_LOAD_H

#include "thumb_instr.h"
#include "../arm7tdmi.h"

void pc_relative_load(arm7tdmi_t* state, thumbinstr_t* instr);

#endif //GBA_PC_RELATIVE_LOAD_H
