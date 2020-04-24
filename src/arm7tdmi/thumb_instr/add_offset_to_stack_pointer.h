#ifndef GBA_ADD_OFFSET_TO_STACK_POINTER_H
#define GBA_ADD_OFFSET_TO_STACK_POINTER_H

#include "../arm7tdmi.h"
#include "thumb_instr.h"

void add_offset_to_stack_pointer(arm7tdmi_t* state, thumbinstr_t* thminstr);
#endif //GBA_ADD_OFFSET_TO_STACK_POINTER_H
