#include "load_address.h"

void load_address(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    load_address_t* instr = &thminstr->LOAD_ADDRESS;
    half offset = instr->word8 << 2;

    if (instr->sp) {
        set_register(state, instr->rd, get_sp(state) + offset);
    } else {
        set_register(state, instr->rd, (state->pc & 0xFFFFFFFD) + offset);
    }
}
