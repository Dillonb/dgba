#ifndef GBA_HIGH_REGISTER_OPERATIONS_H
#define GBA_HIGH_REGISTER_OPERATIONS_H

#include "../arm7tdmi.h"
#include "thumb_instr.h"

void high_register_operations(arm7tdmi_t* state, thumbinstr_t* instr);

#endif //GBA_HIGH_REGISTER_OPERATIONS_H
