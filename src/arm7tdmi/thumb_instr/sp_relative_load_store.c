
#include "sp_relative_load_store.h"
#include "../shifts.h"

void sp_relative_load_store(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    sp_relative_load_store_t* instr = &thminstr->SP_RELATIVE_LOAD_STORE;
    word offset = instr->word8 << 2;
    word address = get_register(state, REG_SP) + offset;
    if (instr->l) {
        word value = state->read_word(address, ACCESS_UNKNOWN);
        value = arm_ror(NULL, value, (address & 3u) << 3);
        set_register(state, instr->rd, value);
    } else {
        state->write_word(address, get_register(state, instr->rd), ACCESS_UNKNOWN);
    }
}
