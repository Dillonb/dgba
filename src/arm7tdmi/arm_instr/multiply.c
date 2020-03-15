#include "multiply.h"
#include "../../common/log.h"

void multiply(arm7tdmi_t* state, multiply_t* instr) {
    unimplemented(instr->rm == instr->rd, "rm must not be the same as rd!")

    unimplemented(instr->rd == 15, "rd must not be 15!")
    unimplemented(instr->rn == 15, "rn must not be 15!")
    unimplemented(instr->rs == 15, "rs must not be 15!")
    unimplemented(instr->rm == 15, "rm must not be 15!")

    uint64_t result = get_register(state, instr->rm);
    result *= get_register(state, instr->rs);

    if (instr->a) {
        result += get_register(state, instr->rn);
    }

    word wordresult = result & 0xFFFFFFFF;

    unimplemented(instr->s, "Status codes")

    set_register(state, instr->rd, wordresult);
}

void multiply_long(arm7tdmi_t* state, multiply_long_t* instr) {
    unimplemented(instr->s, "status codes")
    uint64_t result;
    if (instr->u) {
        if (instr->a) { // SMLAL
            logfatal("SMLAL: u == 1 && a == 1")
        } else { // SMULL
            logfatal("SMULL: u == 1 && a == 0")
        }
    } else {
        if (instr->a) { // UMLAL
            logfatal("UMLAL: u == 0 && a == 1")
        } else { // UMULL
            logdebug("UMULL: u == 0 && a == 0")
            result = get_register(state, instr->rm);
            result *= get_register(state, instr->rs);
        }
    }

    word high = (result >> 32u) & 0xFFFFFFFF;
    word low = result & 0xFFFFFFFF;

    set_register(state, instr->rdlo, low);
    set_register(state, instr->rdhi, high);
}
