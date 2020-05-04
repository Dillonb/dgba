#include "block_data_transfer.h"
#include "../../common/log.h"

void block_data_transfer(arm7tdmi_t* state, arminstr_t * arminstr) {
    block_data_transfer_t* instr = &arminstr->parsed.BLOCK_DATA_TRANSFER;
    word address = get_register(state, instr->rn);
    word base = address;
    word new_base = base;

    int num_registers = popcount(instr->rlist);

    bool p = instr->p;
    bool w = instr->w;

    byte original_mode = state->cpsr.mode;
    if (instr->s) {
        state->cpsr.mode = MODE_USER;
    }

    if (instr->rlist == 0u) {
        // Weird stuff happens when you don't specify any registers to transfer
        if (instr->l) {
            set_pc(state, state->read_word(address));
        } else {
            if (instr->u) {
                if (p) {
                    // 11 => STMIB
                    state->write_word(base + 4, get_register(state, REG_PC) + 4);
                } else {
                    // 10 => STMIA
                    state->write_word(base, get_register(state, REG_PC) + 4);
                }
            } else {
                if (p) {
                    // 01 => STMDB
                    state->write_word(base - 0x40, get_register(state, REG_PC) + 4);
                } else {
                    // 00 => STMDA
                    state->write_word(base - 0x3C, get_register(state, REG_PC) + 4);
                }
            }
        }
        address = instr->u ? address + 0x40 : address - 0x40;
    } else {
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
            new_base = address;
            if (w) {
                set_register(state, instr->rn, address);
                w = false;
            }
        } else {
            new_base = address + 4 * num_registers;
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

        if (instr->l) {
            for (unsigned int rt = 0; rt <= REG_PC; rt++) {
                if ((instr->rlist >> rt & 1) == 1) {
                    if (instr->rlist & (1 << instr->rn)) {
                        w = false; // Don't writeback to rn when we're also transferring to rn
                    }
                    logdebug("Will transfer r%d\n", rt);
                    address += before_inc;
                    logdebug("Transferring 0x%08X to r%d", address, rt)
                    set_register(state, rt, state->read_word(address));
                    address += after_inc;
                }
            }
        } else {
            bool first = true;
            for (unsigned int rt = 0; rt <= REG_PC; rt++) {
                if ((instr->rlist >> rt & 1) == 1) {
                    logdebug("Will transfer r%d\n", rt);
                    address += before_inc;
                    logdebug("Transferring r%d to 0x%08X", rt, address)
                    word value;
                    if (rt == instr->rn) {
                        if (first) {
                            // Base first in rlist: write back old base.
                            value = base;
                        } else {
                            // Base NOT first in rlist: write back NEW base (address that gets written back)
                            value = new_base;
                        }
                    } else {
                        value = get_register(state, rt);
                        if (rt == REG_PC) {
                            value += 4;
                        }
                    }
                    state->write_word(address, value);
                    first = false;
                    address += after_inc;
                }
            }
        }
    }

    // If we're growing down, we wrote back at the beginning, and this w will be false.
    if (w) {
        set_register(state, instr->rn, address);
    }

    if (instr->s) {
        state->cpsr.mode = original_mode;
    }
}
