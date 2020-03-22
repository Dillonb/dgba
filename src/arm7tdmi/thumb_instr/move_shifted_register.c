#include "move_shifted_register.h"
#include "../../common/log.h"
#include "../shifts.h"

void move_shifted_register(arm7tdmi_t* state, move_shifted_register_t* instr) {
    switch (instr->opcode) {
        case 0: { // LSL
            word value = get_register(state, instr->rs);
            if (instr->offset == 0) {
                // No shift performed and carry flag not updated
                value = arm_shift_special_zero_behavior(state, &state->cpsr, LSL, value);
            } else {
                value = arm_shift(&state->cpsr, LSL, value, instr->offset);
            }
            set_register(state, instr->rd, value);
            set_flags_nz(state, value);
            break;
        }
        case 1: { // LSR
            word value = get_register(state, instr->rs);
            if (instr->offset == 0) {
                // No shift performed and carry flag not updated
                value = arm_shift_special_zero_behavior(state, &state->cpsr, LSR, value);
            } else {
                value = arm_shift(&state->cpsr, LSR, value, instr->offset);
            }
            set_register(state, instr->rd, value);
            set_flags_nz(state, value);
            break;
        }
        case 2: { // ASR
            word value = get_register(state, instr->rs);
            if (instr->offset == 0) {
                // No shift performed and carry flag not updated
                value = arm_shift_special_zero_behavior(state, &state->cpsr, ASR, value);
            } else {
                value = arm_shift(&state->cpsr, ASR, value, instr->offset);
            }
            set_register(state, instr->rd, value);
            set_flags_nz(state, value);
            break;
        }
        default:
            logfatal("Unknown MSR opcode: %d", instr->opcode)
    }
}
