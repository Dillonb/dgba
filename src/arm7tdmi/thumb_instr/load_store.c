#include "load_store.h"
#include "../../common/log.h"
#include "../shifts.h"

void load_store_ro(arm7tdmi_t* state, load_store_ro_t* instr) {
    word address = get_register(state, instr->rb) + get_register(state, instr->ro);
    if (instr->l) {
        if (instr->b) {
            word value = state->read_byte(address);
            set_register(state, instr->rd, value);
        } else {
            word value = state->read_word(address);
            value = arm_ror(NULL, value, (address & 3u) << 3);
            set_register(state, instr->rd, value);
        }
    } else {
        word value = get_register(state, instr->rd);
        if (instr->b) {
            byte bytevalue = value & 0xFF;
            state->write_byte(address, bytevalue);
        } else {
            state->write_word(address, value);
        }
    }
}
void load_store_io(arm7tdmi_t* state, load_store_io_t* instr) {
    logfatal("load_store_io")
}
