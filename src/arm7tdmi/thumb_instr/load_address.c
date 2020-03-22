#include "load_address.h"
#include "../../common/log.h"

void load_address(arm7tdmi_t* state, load_address_t* instr) {
    unimplemented(instr->sp == 1, "Loading an address to the SP")

    half offset = instr->word8 << 2;

    if (instr->sp) {
        set_register(state, instr->rd, get_sp(state) + offset);
    } else {
        set_register(state, instr->rd, (state->pc & 0xFFFFFFFD) + offset);
    }
}
