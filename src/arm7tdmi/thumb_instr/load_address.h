#ifndef GBA_LOAD_ADDRESS_H
#define GBA_LOAD_ADDRESS_H

#include "../arm7tdmi.h"
#include "thumb_instr.h"

void load_address(arm7tdmi_t* state, thumbinstr_t* instr);

#endif //GBA_LOAD_ADDRESS_H
