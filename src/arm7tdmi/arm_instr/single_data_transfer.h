#ifndef __GBA_SINGLE_DATA_TRANSFER_H__
#define __GBA_SINGLE_DATA_TRANSFER_H__

#include <stdbool.h>
#include "../arm7tdmi.h"

void single_data_transfer(arm7tdmi_t* state,
                          unsigned int offset,
                          unsigned int rd, // dest if this is LDR, source if this is STR
                          unsigned int rn,
                          bool l,
                          bool w,
                          bool b,
                          bool up,
                          bool pre,
                          bool immediate_offset_type);
#endif
