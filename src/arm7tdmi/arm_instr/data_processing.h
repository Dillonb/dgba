//
// Created by Dillon Beliveau on 2/25/20.
//

#ifndef GBA_DATA_PROCESSING_H
#define GBA_DATA_PROCESSING_H

#include <stdbool.h>
#include "../arm7tdmi.h"

void data_processing(arm7tdmi_t* state, word immediate_operand2, word rd, word rn, bool s, bool immediate, word opcode);

#endif //GBA_DATA_PROCESSING_H
