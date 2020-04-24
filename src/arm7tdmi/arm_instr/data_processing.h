#ifndef GBA_DATA_PROCESSING_H
#define GBA_DATA_PROCESSING_H

#include <stdbool.h>
#include "../arm7tdmi.h"
#include "arm_instr.h"

void data_processing(arm7tdmi_t* state, arminstr_t* arminstr);

#endif //GBA_DATA_PROCESSING_H
