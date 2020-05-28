#include "push_pop_registers.h"

void push_pop_registers(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    push_pop_registers_t* instr = &thminstr->PUSH_POP_REGISTERS;
    half rlist = instr->rlist;
    if (instr->r) {
        if (instr->l) {
            rlist |= (1 << REG_PC);
        } else {
            rlist |= (1 << REG_LR);
        }
    }

    int num_registers = popcount(rlist);

    if (instr->l) { // Pop
        for (int i = 0; i <= 15; i++) {
            if ((rlist >> i) & 1) {
                word sp = get_register(state, REG_SP);
                word value = state->read_word(sp, ACCESS_UNKNOWN);
                if (i == REG_PC) {
                    value |= 1;
                }
                set_register(state, i, value);
                set_register(state, REG_SP, sp + sizeof(word));
            }
        }
    } else { // Push
        set_register(state, REG_SP, get_register(state, REG_SP) - sizeof(word) * num_registers);
        word addr = get_register(state, REG_SP);

        for (int i = 0; i <= 15; i++) {
            if ((rlist >> i) & 1) {
                state->write_word(addr, get_register(state, i), ACCESS_UNKNOWN);
                addr += sizeof(word);
            }
        }
    }
}
