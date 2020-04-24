#ifndef GBA_MOVE_SHIFTED_REGISTER_H
#define GBA_MOVE_SHIFTED_REGISTER_H

#include "../arm7tdmi.h"
#include "thumb_instr.h"

void move_shifted_register(arm7tdmi_t* state, thumbinstr_t* instr);

#endif //GBA_MOVE_SHIFTED_REGISTER_H
