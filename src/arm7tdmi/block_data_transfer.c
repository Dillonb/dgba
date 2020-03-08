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
    unimplemented(rlist == 0, "special case when rlist == 0")

    word address = get_register(state, rn);

    byte original_mode = state->cpsr.mode;
    if (u) {
        state->cpsr.mode = MODE_USER;
    }

    for (unsigned int rt = 0; rt <= 15; rt++) {
        if ((rlist >> rt & 1) == 1) {
            unimplemented(rt == rn, "transferring rn in block data transfer")
            printf("Will transfer r%d\n", rt);
            if (p) { address += 8; }
            logdebug("Transferring r%d to 0x%08X", rt, address)
            state->write_word(address, get_register(state, rt));
            if (!p) { address += 8; }
        }
    }

    if (u) {
        state->cpsr.mode = original_mode;
    }
    logfatal("rlist: 0x%X rn: %X l: %d w: %d s: %d u: %d p: %d", rlist, rn, l, w, s, u, p)
}
