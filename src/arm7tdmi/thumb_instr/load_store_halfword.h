#ifndef GBA_LOAD_STORE_HALFWORD_H
#define GBA_LOAD_STORE_HALFWORD_H

#include "thumb_instr.h"
#include "../arm7tdmi.h"

void load_store_halfword(arm7tdmi_t* state, thumbinstr_t* instr);
void load_store_byte_halfword(arm7tdmi_t* state, thumbinstr_t* instr);

#endif //GBA_LOAD_STORE_HALFWORD_H
