#include "add_offset_to_stack_pointer.h"
#include "../../common/log.h"

void add_offset_to_stack_pointer(arm7tdmi_t* state, add_offset_to_stack_pointer_t* instr) {
    word offset = instr->sword7 << 2;
    word newsp = get_register(state, 13);
    if (instr->s) {
        newsp -= offset;
    } else {
        newsp += offset;
    }
    set_register(state, 13, newsp);

}
