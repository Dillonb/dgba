#include "conditional_branch.h"
#include "../sign_extension.h"

void conditional_branch(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    conditional_branch_t* instr = &thminstr->CONDITIONAL_BRANCH;
    bool passed;
    switch (instr->cond) {
        case EQ: passed = state->cpsr.Z == 1; break;
        case NE: passed = state->cpsr.Z == 0; break;
        case CS: passed = state->cpsr.C == 1; break;
        case CC: passed = state->cpsr.C == 0; break;
        case MI: passed = state->cpsr.N == 1; break;
        case PL: passed = state->cpsr.N == 0; break;
        case VS: passed = state->cpsr.V == 1; break;
        case VC: passed = state->cpsr.V == 0; break;
        case HI: passed = state->cpsr.C == 1 && state->cpsr.Z == 0; break;
        case LS: passed = state->cpsr.C == 0 || state->cpsr.Z == 1; break;
        case GE: passed = (!state->cpsr.N == !state->cpsr.V); break;
        case LT: passed = (!state->cpsr.N != !state->cpsr.V); break;
        case GT: passed = (!state->cpsr.Z && !state->cpsr.N == !state->cpsr.V); break;
        case LE: passed = (state->cpsr.Z || !state->cpsr.N != !state->cpsr.V); break;
        case AL: logfatal("AL case used for COND in thumb mode! This is undefined and control should not have gotten here!")
        case NV: logfatal("NV case used for COND in thumb mode! This is actually a SWI instruction!")
        default: logfatal("Unimplemented COND: %d", instr->cond)
    }

    if (passed) {
        word offset = instr->soffset;
        offset = sign_extend_word(offset, 8, 32);
        offset <<= 1;
        int signed_offset = (int)offset;
        word newpc = state->pc + signed_offset;
        newpc |= 1; // Set thumb mode bit
        set_pc(state, newpc);
    }
}
