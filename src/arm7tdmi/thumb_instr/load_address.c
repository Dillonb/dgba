//
// Created by dillon on 3/10/20.
//

#include "load_address.h"
#include "../../common/log.h"

void load_address(arm7tdmi_t* state, load_address_t* instr) {
    unimplemented(instr->sp == 1, "Loading an address to the SP")

    if (instr->sp) {
        set_register(state, instr->rd, get_sp(state) + instr->word8);
    } else {
        set_register(state, instr->rd, state->pc + instr->word8);
    }
}
