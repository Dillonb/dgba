#ifndef GBA_MULTIPLE_LOAD_STORE_H
#define GBA_MULTIPLE_LOAD_STORE_H

#include "../arm7tdmi.h"
#include "thumb_instr.h"

void multiple_load_store(arm7tdmi_t* state, thumbinstr_t* instr);
#endif //GBA_MULTIPLE_LOAD_STORE_H
