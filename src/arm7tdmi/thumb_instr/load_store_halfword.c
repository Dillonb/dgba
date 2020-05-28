#include "load_store_halfword.h"
#include "../sign_extension.h"
#include "../shifts.h"

void load_store_halfword(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    load_store_halfword_t* instr = &thminstr->LOAD_STORE_HALFWORD;
    word addr = get_register(state, instr->rb) + (instr->offset << 1);
    if (instr->l) {
        word value = state->read_half(addr, ACCESS_UNKNOWN);
        value = arm_ror(NULL, value, (addr & 0x1) << 3);
        set_register(state, instr->rd, value);
    } else {
        half value = get_register(state, instr->rd) & 0xFFFF;
        state->write_half(addr, value, ACCESS_UNKNOWN);
    }
}

void load_store_byte_halfword(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    load_store_byte_halfword_t* instr = &thminstr->LOAD_STORE_BYTE_HALFWORD;
    word address = get_register(state, instr->rb) + get_register(state, instr->ro);
    if (instr->s == 0 && instr->h == 0) {
        half rddata = get_register(state, instr->rd) & 0xFFFF;
        state->write_half(address, rddata, ACCESS_UNKNOWN);
    } else {
        word value;
        if (instr->h) {
            if (instr->s) {
                if (address & 1) {
                    value = state->read_byte(address, ACCESS_UNKNOWN);
                    value = sign_extend_word(value, 8, 32);
                } else {
                    value = state->read_half(address, ACCESS_UNKNOWN);
                    value = sign_extend_word(value, 16, 32);
                }
            } else {
                value = state->read_half(address, ACCESS_UNKNOWN);
                value = arm_ror(NULL, value, (address & 0x1) << 3);
            }
        } else {
            value = state->read_byte(address, ACCESS_UNKNOWN);
            value = sign_extend_word(value, 8, 32);
        }

        set_register(state, instr->rd, value);
    }
}
