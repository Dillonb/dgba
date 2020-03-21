#include "unconditional_branch.h"
#include "../sign_extension.h"

void unconditional_branch(arm7tdmi_t* state, unconditional_branch_t* instr) {
    int offset = sign_extend_word(instr->offset << 1, 12, 32);
    word newpc = state->pc + offset;
    newpc |= 1; // Set thumb mode bit
    set_pc(state, newpc);
}
