//
// Created by dillon on 3/10/20.
//

#ifndef GBA_HIGH_REGISTER_OPERATIONS_H
#define GBA_HIGH_REGISTER_OPERATIONS_H

#include "../arm7tdmi.h"

void high_register_operations(arm7tdmi_t* state, byte opcode, bool h1, bool h2, byte rshs, byte rdhd);

#endif //GBA_HIGH_REGISTER_OPERATIONS_H
