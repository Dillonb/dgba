#include <stddef.h>
#include "sp_relative_load_store.h"
#include "../shifts.h"

void sp_relative_load_store(arm7tdmi_t* state, sp_relative_load_store_t* instr) {
    word offset = instr->word8 << 2;
    word address = get_register(state, REG_SP) + offset;
    if (instr->l) {
        word value = state->read_word(address);
        value = arm_ror(NULL, value, (address & 3u) << 3);
        set_register(state, instr->rd, value);
    } else {
        state->write_word(address, get_register(state, instr->rd));
    }
}
