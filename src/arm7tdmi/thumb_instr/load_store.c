#include "load_store.h"
#include "../shifts.h"

void load_store_ro(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    load_store_ro_t* instr = &thminstr->LOAD_STORE_RO;
    word address = get_register(state, instr->rb) + get_register(state, instr->ro);
    if (instr->l) {
        if (instr->b) {
            word value = state->read_byte(address, ACCESS_UNKNOWN);
            set_register(state, instr->rd, value);
        } else {
            word value = state->read_word(address, ACCESS_UNKNOWN);
            value = arm_ror(NULL, value, (address & 3u) << 3);
            set_register(state, instr->rd, value);
        }
    } else {
        word value = get_register(state, instr->rd);
        if (instr->b) {
            byte bytevalue = value & 0xFF;
            state->write_byte(address, bytevalue, ACCESS_UNKNOWN);
        } else {
            state->write_word(address, value, ACCESS_UNKNOWN);
        }
    }
}

void load_store_io(arm7tdmi_t* state, thumbinstr_t * thminstr) {
    load_store_io_t* instr = &thminstr->LOAD_STORE_IO;
    word offset = instr->offset << (instr->b ? 0 : 2);
    word address = get_register(state, instr->rb) + offset;
    if (instr->l) {
        if (instr->b) {
            word value = state->read_byte(address, ACCESS_UNKNOWN);
            set_register(state, instr->rd, value);
        } else {
            word value = state->read_word(address, ACCESS_UNKNOWN);
            value = arm_ror(NULL, value, (address & 3u) << 3);
            set_register(state, instr->rd, value);
        }
    } else {
        word value = get_register(state, instr->rd);
        if (instr->b) {
            byte bytevalue = value & 0xFF;
            state->write_byte(address, bytevalue, ACCESS_UNKNOWN);
        } else {
            state->write_word(address, value, ACCESS_UNKNOWN);
        }
    }
}
