#include <stdbool.h>
#include "arm7tdmi.h"
#include "../common/log.h"

void branch_exchange(arm7tdmi_t* state, byte opcode, byte rn) {
    switch (opcode) {
        case 0b0001: {// BX
            word rndata = get_register(state, rn);
            bool thumb = rndata & 1u;
            unimplemented(thumb, "THUMB mode unimplemented")
            state->pc = rndata;
            break;
        }
        case 0b0010: // BXJ
            logfatal("BXJ: This implementation does not support Jazelle mode!")
        case 0b0011: // BLX
            logfatal("BLX unimplemented")
        default:
            logfatal("BRANCH_EXCHANGE: Unimplemented opcode %d", opcode)

    }

}

void branch(arm7tdmi_t* state, word offset, bool link) {
    bool thumb = offset & 1u;
    unimplemented(thumb, "THUMB mode")
    bool negative = (offset & 0x800000u) > 0;
    if (negative) {
        // Since this is a 24 bit value stored in a 32 bit int, need to mask away the top 8 bits
        // after doing the two's complement thingy
        offset = (~offset + 1) & 0xFFFFFFu;
    }
    offset <<= 2u;

    int signed_offset = negative ? -1 * (int)offset : (int)offset;

    loginfo("My offset is %d", signed_offset)

    if (link) {
        word returnaddress = state->pc - 4; // PC is 3 bytes ahead, need to make it just 1 byte ahead for returning.
        state->lr = returnaddress;
    }

    word newpc = (state->pc) + signed_offset;
    logdebug("Hold on to your hats, we're jumping to 0x%02X", newpc)
    set_pc(state, newpc);
}

