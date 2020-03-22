#include "long_branch_link.h"
#include "../sign_extension.h"

void long_branch_link(arm7tdmi_t* state, long_branch_link_t* instr) {
    word offset = instr->offset;

    if (instr->h) {
        offset <<= 1;
        word oldlr = get_register(state, REG_LR);
        word oldpc = get_register(state, REG_PC);
        word full_address = oldlr + offset + 1;
        set_register(state, REG_LR, (oldpc - 2) | 1);
        set_register(state, REG_PC, full_address);
    } else {
        offset = sign_extend_word(offset, 11, 32);
        offset <<= 12;
        set_register(state, REG_LR, state->pc + offset);
    }

}
