#include <stdbool.h>
#include "branch.h"
#include "../arm7tdmi.h"
#include "../../common/log.h"

void branch_exchange(arm7tdmi_t* state, arminstr_t * arminstr) {
    branch_exchange_t* instr = &arminstr->parsed.BRANCH_EXCHANGE;
    switch (instr->opcode) {
        case 0b0001: {// BX
            word newpc = get_register(state, instr->rn);
            bool thumb = newpc & 1u;
            logdebug("[BX] Hold on to your hats, we're jumping to 0x%02X - from r%d", newpc, instr->rn)
            if (thumb) logdebug("REALLY hang on, we're entering THUMB mode!")
            set_pc(state, newpc);
            break;
        }
        case 0b0010: // BXJ
            logfatal("BXJ: This implementation does not support Jazelle mode!")
        case 0b0011: // BLX
            logfatal("BLX unimplemented")
        default:
            logfatal("BRANCH_EXCHANGE: Unimplemented opcode %d", instr->opcode)

    }

}

void branch(arm7tdmi_t* state, arminstr_t * arminstr) {
    branch_t* instr = &arminstr->parsed.BRANCH;
    bool negative = (instr->offset & 0x800000u) > 0;

    word offset = instr->offset;

    if (negative) {
        // Since this is a 24 bit value stored in a 32 bit int, need to mask away the top 8 bits
        // after doing the two's complement thingy
        offset = (~instr->offset + 1u) & 0xFFFFFFu;
    }
    offset <<= 2u; // This means we can never enter thumb mode through this instruction

    int signed_offset = negative ? -1 * (int)offset : (int)offset;

    if (instr->link) {
        word returnaddress = state->pc - 4; // PC is 3 bytes ahead, need to make it just 1 byte ahead for returning.
        state->lr = returnaddress;
    }

    word newpc = (state->pc) + signed_offset;
    logdebug("[B] Hold on to your hats, we're jumping to 0x%02X", newpc)
    set_pc(state, newpc);
}

