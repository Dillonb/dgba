#include "load_store_halfword.h"
#include "../../common/log.h"

void load_store_halfword(arm7tdmi_t* state, load_store_halfword_t* instr) {
    word addr = get_register(state, instr->rb) + (instr->offset << 1);
    if (instr->l) {
        word value = state->read_half(addr);
        set_register(state, instr->rd, value);
    } else {
        half value = get_register(state, instr->rd) & 0xFFFF;
        state->write_half(addr, value);
    }
}
void load_store_byte_halfword(arm7tdmi_t* state, load_store_byte_halfword_t* instr) {
    logfatal("load_store_byte_halfword")
}
