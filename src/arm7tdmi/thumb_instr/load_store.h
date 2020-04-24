#ifndef GBA_LOAD_STORE_H
#define GBA_LOAD_STORE_H
#include "../arm7tdmi.h"
#include "thumb_instr.h"

void load_store_ro(arm7tdmi_t* state, thumbinstr_t* instr);
void load_store_io(arm7tdmi_t* state, thumbinstr_t* instr);

#endif //GBA_LOAD_STORE_H
