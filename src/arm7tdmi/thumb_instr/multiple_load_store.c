#include "multiple_load_store.h"

void multiple_load_store(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    multiple_load_store_t* instr = &thminstr->MULTIPLE_LOAD_STORE;
    word address = get_register(state, instr->rb);
    word base = address;
    bool writeback = true;
    if (instr->rlist == 0) { // When rlist 0, save and load the program counter instead
        if (instr->l) {
            set_register(state, REG_PC, state->read_word(address) | 1);
        } else {
            state->write_word(address, state->pc + 2);
        }
        address += 0x40;
    }
    else if (instr->l) {
        if (instr->rlist & (1 << instr->rb)) {
            writeback = false; // Don't writeback to rb when we're also transferring to rb
        }
        for (int i = 0; i <= 8; i++) {
            if ((instr->rlist >> i) & 1) {
                set_register(state, i, state->read_word(address));
                address += 4;
            }
        }
    } else {
        for (int i = 0; i <= 8; i++) {
            if ((instr->rlist >> i) & 1) {
                word value;
                if (i == instr->rb) {
                    value = base;
                } else {
                    value = get_register(state, i);
                }

                state->write_word(address, value);
                address += 4;
            }
        }
    }

    if (writeback) {
        set_register(state, instr->rb, address);
    }
}
