#include "pc_relative_load.h"

void pc_relative_load(arm7tdmi_t *state, thumbinstr_t* thminstr) {
    pc_relative_load_t* instr = &thminstr->PC_RELATIVE_LOAD;
    word offset = instr->word8 << 2;
    word maskedpc = state->pc & 0xFFFFFFFC;

    word address = maskedpc + offset;

    set_register(state, instr->rd, state->read_word(address, ACCESS_NONSEQUENTIAL));
}
