#include "multiply.h"
#include "../sign_extension.h"

void multiply(arm7tdmi_t* state, arminstr_t* arminstr) {
    multiply_t* instr = &arminstr->parsed.MULTIPLY;
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

    if (instr->s) {
        set_flags_nz(state, wordresult);
    }

    set_register(state, instr->rd, wordresult);
}

void multiply_long(arm7tdmi_t* state, arminstr_t* arminstr) {
    multiply_long_t* instr = &arminstr->parsed.MULTIPLY_LONG;
    uint64_t rmdata = get_register(state, instr->rm);
    uint64_t rsdata = get_register(state, instr->rs);
    uint64_t result;
    if (instr->u) {
        rmdata = sign_extend_64(rmdata, 32, 64);
        rsdata = sign_extend_64(rsdata, 32, 64);
    }

    result = rmdata * rsdata;

    if (instr->a) { // accumulate (add on to what's already set)
        uint64_t existing = get_register(state, instr->rdhi);
        existing <<= 32;
        existing |= get_register(state, instr->rdlo) & 0xFFFFFFFF;
        result += existing;
    }

    if (instr->s) {
        status_register_t* psr = get_psr(state);
        psr->Z = result == 0;
        psr->N = result >> 63;
    }

    word high = (result >> 32u) & 0xFFFFFFFF;
    word low = result & 0xFFFFFFFF;

    set_register(state, instr->rdlo, low);
    set_register(state, instr->rdhi, high);
}
