#include "conditional_branch.h"
#include "../../common/log.h"

void conditional_branch(arm7tdmi_t* state, conditional_branch_t* instr) {
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
        int8_t offset = (int8_t)instr->soffset; // Store it in a signed datatype
        if (offset < 0) {
            logfatal("Negative offset!")
        }
        set_pc(state, state->pc + offset);
    }
}
