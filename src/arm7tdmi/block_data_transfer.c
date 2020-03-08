//
// Created by dillon on 3/7/20.
//

#include "block_data_transfer.h"
#include "../common/log.h"

void block_data_transfer(arm7tdmi_t* state,
                         word rlist,
                         word rn,
                         bool l,
                         bool w,
                         bool s,
                         bool u,
                         bool p) {
    unimplemented(l, "block data transfer: load from memory")
    unimplemented(w, "block data transfer: write back")
    unimplemented(!s, "block data transfer: s flag off")
    unimplemented(!u, "block data transfer: subtract offset from base")
    unimplemented(p, "block data transfer: add offset pre-transfer")





    logfatal("rlist: 0x%X rn: %X l: %d w: %d s: %d u: %d p: %d", rlist, rn, l, w, s, u, p)
}
