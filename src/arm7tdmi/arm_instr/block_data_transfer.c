#include "block_data_transfer.h"
#include "../../common/log.h"

void block_data_transfer(arm7tdmi_t* state, block_data_transfer_t* instr) {
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

        // Also, since we'll be doing everything in reverse, do the writeback operation now,
        // and flip the p flag.
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

    if (instr->l) {
        for (unsigned int rt = 0; rt <= 15; rt++) {
            if ((instr->rlist >> rt & 1) == 1) {
                unimplemented(rt == instr->rn, "transferring rn in block data transfer")
                printf("Will transfer r%d\n", rt);
                address += before_inc;
                logdebug("Transferring 0x%08X to r%d", address, rt)
                set_register(state, rt, state->read_word(address));
                address += after_inc;
            }
        }
    } else {
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
    }

    // If we're growing up, that means we just hit the highest address, and should writeback if that flag is also set.
    // If we're growing down, we wrote back at the beginning.
    if (instr->u && instr->w) {
        set_register(state, instr->rn, address);
    }

    if (instr->s) {
        state->cpsr.mode = original_mode;
    }
}
