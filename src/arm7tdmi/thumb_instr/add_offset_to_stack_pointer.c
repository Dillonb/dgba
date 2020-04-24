#include "add_offset_to_stack_pointer.h"
#include "../../common/log.h"

void add_offset_to_stack_pointer(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    add_offset_to_stack_pointer_t* instr = &thminstr->ADD_OFFSET_TO_STACK_POINTER;
    word offset = instr->sword7 << 2;
    word newsp = get_register(state, 13);
    if (instr->s) {
        newsp -= offset;
    } else {
        newsp += offset;
    }
    set_register(state, 13, newsp);

}
