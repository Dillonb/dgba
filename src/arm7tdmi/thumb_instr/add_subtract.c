#include "add_subtract.h"

void add_subtract(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    add_subtract_t* instr = &thminstr->ADD_SUBTRACT;
    word newvalue = get_register(state, instr->rs);
    if (instr->op) { // subtract
        word result = newvalue;
        if (instr->i) {
            result -= instr->rn_or_offset;
            set_flags_sub(state, newvalue, instr->rn_or_offset, result);
        } else {
            word rndata = get_register(state, instr->rn_or_offset);
            result -= rndata;
            set_flags_sub(state, newvalue, rndata, result);
        }
        newvalue = result;
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
