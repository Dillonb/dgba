//
// Created by dillon on 3/7/20.
//

#ifndef GBA_BLOCK_DATA_TRANSFER_H
#define GBA_BLOCK_DATA_TRANSFER_H

#include <stdbool.h>
#include "../arm7tdmi.h"

void block_data_transfer(arm7tdmi_t* state,
                         word rlist,
                         word rn,
                         bool l,
                         bool w,
                         bool s,
                         bool u,
                         bool p);

#endif //GBA_BLOCK_DATA_TRANSFER_H
