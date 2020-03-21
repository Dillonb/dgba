#include "add_subtract.h"
#include "../../common/log.h"

void add_subtract(arm7tdmi_t* state, add_subtract_t* instr) {
    word newvalue = get_register(state, instr->rs);
    if (instr->op) { // subtract
        logfatal("subtract")
    } else { // add
        if (instr->i) {
            set_flags_add(state, newvalue, instr->rn_or_offset);
            newvalue += instr->rn_or_offset;
        } else {
            word rndata = get_register(state, instr->rn_or_offset);
            set_flags_add(state, newvalue, rndata);
            newvalue += get_register(state, instr->rn_or_offset);
        }
    }

    set_flags_nz(state, newvalue);
    set_register(state, instr->rd, newvalue);
}
