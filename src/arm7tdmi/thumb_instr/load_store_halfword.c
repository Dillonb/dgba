#include "load_store_halfword.h"
#include "../../common/log.h"
#include "../sign_extension.h"

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
    word address = get_register(state, instr->rb) + get_register(state, instr->ro);
    if (instr->s == 0 && instr->h == 0) {
        half rddata = get_register(state, instr->rd) & 0xFFFF;
        state->write_half(address, rddata);
    } else {
        word value;
        if (instr->h) {
            value = state->read_half(address);
        } else {
            value = state->read_byte(address);
        }

        if (instr->s) {
            value = sign_extend_word(value, instr->h ? 16 : 8, 32);
        }

        set_register(state, instr->rd, value);
    }
}
