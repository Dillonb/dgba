#ifndef GBA_SP_RELATIVE_LOAD_STORE_H
#define GBA_SP_RELATIVE_LOAD_STORE_H

#include "thumb_instr.h"
#include "../arm7tdmi.h"

void sp_relative_load_store(arm7tdmi_t* state, sp_relative_load_store_t* instr);
#endif //GBA_SP_RELATIVE_LOAD_STORE_H
