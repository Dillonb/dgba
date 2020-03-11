//
// Created by dillon on 3/10/20.
//

#ifndef GBA_HIGH_REGISTER_OPERATIONS_H
#define GBA_HIGH_REGISTER_OPERATIONS_H

#include "../arm7tdmi.h"
#include "thumb_instr.h"

void high_register_operations(arm7tdmi_t* state, high_register_operations_t* instr);

#endif //GBA_HIGH_REGISTER_OPERATIONS_H
