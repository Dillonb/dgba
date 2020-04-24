#include "unconditional_branch.h"
#include "../sign_extension.h"

void unconditional_branch(arm7tdmi_t* state, thumbinstr_t* instr) {
    int offset = sign_extend_word(instr->UNCONDITIONAL_BRANCH.offset << 1, 12, 32);
    word newpc = state->pc + offset;
    newpc |= 1; // Set thumb mode bit
    set_pc(state, newpc);
}
