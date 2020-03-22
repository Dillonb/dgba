#include "pc_relative_load.h"

void pc_relative_load(arm7tdmi_t *state, pc_relative_load_t *instr) {
    word offset = instr->word8 << 2;
    word maskedpc = state->pc & 0xFFFFFFFC;

    word address = maskedpc + offset;

    set_register(state, instr->rd, state->read_word(address));
}
