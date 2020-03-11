#include "block_data_transfer.h"
#include "../../common/log.h"

void block_data_transfer(arm7tdmi_t* state, block_data_transfer_t* instr) {
    unimplemented(instr->l, "block data transfer: load from memory")
    unimplemented(instr->u && instr->w, "block data transfer: write back when u set")
    unimplemented(instr->rlist == 0, "special case when rlist == 0")

    word address = get_register(state, instr->rn);

    int num_registers = popcount(instr->rlist);

    bool p = instr->p;

    if (!instr->u) {
        // When the u flag is 0, we grow down from the base register.
        // The CPU, however, still saves from the lowest numbered register first.
        // This is important when we're writing to memory-mapped io registers.
        // We simulate this behavior by setting the base to where it should _end_,
        // and growing upwards.
        // Also, flip the p flag since we'll be doing this in reverse.
        p = !p;
        address -= 4 * num_registers;
        if (instr->w) {
            set_register(state, instr->rn, address);
        }
    }

    int before_inc;
    int after_inc;
    if (p) {
        before_inc = 4;
        after_inc = 0;
    } else {
        before_inc = 0;
        after_inc = 4;
    }


    byte original_mode = state->cpsr.mode;
    if (instr->s) {
        state->cpsr.mode = MODE_USER;
    }

    for (unsigned int rt = 0; rt <= 15; rt++) {
        if ((instr->rlist >> rt & 1) == 1) {
            unimplemented(rt == instr->rn, "transferring rn in block data transfer")
            printf("Will transfer r%d\n", rt);
            address += before_inc;
            logdebug("Transferring r%d to 0x%08X", rt, address)
            state->write_word(address, get_register(state, rt));
            address += after_inc;
        }
    }

    if (instr->s) {
        state->cpsr.mode = original_mode;
    }
}
