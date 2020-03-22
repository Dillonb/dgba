#include "alu_operations.h"
#include "../../common/log.h"
#include "../shifts.h"

void alu_operations(arm7tdmi_t* state, alu_operations_t* instr) {
    switch (instr->opcode) {
        case 0x0: { // AND: Rd = Rd AND Rs
            word newvalue = get_register(state, instr->rd) & get_register(state, instr->rs);
            set_flags_nz(state, newvalue);
            set_register(state, instr->rd, newvalue);
            break;
        }
        case 0x1: { // EOR: Rd = Rd ^ Rs
            word newvalue = get_register(state, instr->rd) ^ get_register(state, instr->rs);
            set_flags_nz(state, newvalue);
            set_register(state, instr->rd, newvalue);
            break;
        }
        case 0x2: { // LSL: Rd = Rd << Rs
            word newvalue = get_register(state, instr->rd);
            word shift_amount = get_register(state, instr->rs);
            newvalue = arm_shift(&state->cpsr, LSL, newvalue, shift_amount);
            set_flags_nz(state, newvalue);
            set_register(state, instr->rd, newvalue);
            break;
        }
        case 0x3: { // LSR: Rd = Rd >> Rs
            word newvalue = get_register(state, instr->rd);
            word shift_amount = get_register(state, instr->rs);
            newvalue = arm_lsr(&state->cpsr, newvalue, shift_amount);
            set_flags_nz(state, newvalue);
            set_register(state, instr->rd, newvalue);
            break;
        }
        case 0x4: { // ASR: Rd = Rd ASR Rs
            word newvalue = get_register(state, instr->rd);
            word shift_amount = get_register(state, instr->rs);
            newvalue = arm_asr(&state->cpsr, newvalue, shift_amount);
            set_flags_nz(state, newvalue);
            set_register(state, instr->rd, newvalue);
            break;
        }
        case 0x5: { // ADC: Rd = Rd + Rs + C
            uint64_t rddata = get_register(state, instr->rd);
            uint64_t rsdata = get_register(state, instr->rs) + state->cpsr.C;
            word result = rddata + rsdata;
            set_flags_add(state, rddata, rsdata);
            set_flags_nz(state, result);
            set_register(state, instr->rd, result);
            break;
        }
        case 0x6: { // SBC: Rd = Rd - Rs - (~C)
            word rddata = get_register(state, instr->rd);
            word rsdata = get_register(state, instr->rs);
            uint64_t tmp = rsdata;
            tmp += !state->cpsr.C;

            word result = rddata - tmp;

            set_flags_sbc(state, rddata, rsdata, tmp, result);
            set_flags_nz(state, result);
            set_register(state, instr->rd, result);
            break;
        }
        case 0x7: { // ROR: Rd = Rd ROR Rs
            word newvalue = get_register(state, instr->rd);
            word shift_amount = get_register(state, instr->rs);
            newvalue = arm_ror(&state->cpsr, newvalue, shift_amount);
            set_flags_nz(state, newvalue);
            set_register(state, instr->rd, newvalue);
            break;
        }
        case 0x8: { // TST: Set condition codes on Rd AND Rs
            set_flags_nz(state, get_register(state, instr->rd) & get_register(state, instr->rs));
            break;
        }
        case 0x9: { // NEG: Rd = -Rs
            word rsdata = get_register(state, instr->rs);
            word result = 0 - rsdata;
            set_flags_nz(state, result);
            set_flags_sub(state, 0, rsdata, result);
            set_register(state, instr->rd, result);
            break;
        }
        case 0xA: { // CMP: Set condition codes on Rd - Rs
            word rddata = get_register(state, instr->rd);
            word rsdata = get_register(state, instr->rs);
            word result = rddata - rsdata;
            set_flags_nz(state, result);
            set_flags_sub(state, rddata, rsdata, result);
            break;
        }
        case 0xB: { // CMN: Set condition codes on Rd + Rs
            set_flags_add(state, get_register(state, instr->rd), get_register(state, instr->rs));
            set_flags_nz(state, get_register(state, instr->rd) + get_register(state, instr->rs));
            break;
        }
        case 0xC: { // ORR: Rd = Rd | Rs
            word newvalue = get_register(state, instr->rd) | get_register(state, instr->rs);
            set_flags_nz(state, newvalue);
            set_register(state, instr->rd, newvalue);
            break;
        }
        case 0xD: { // MUL: Rd = Rd * Rs
            word result = get_register(state, instr->rd) * get_register(state, instr->rs);
            set_flags_nz(state, result);
            set_register(state, instr->rd, result);
            break;
        }
        case 0xE: { // BIC: Rd = Rd & ~Rs
            word newvalue = get_register(state, instr->rd) & ~get_register(state, instr->rs);
            set_flags_nz(state, newvalue);
            set_register(state, instr->rd, newvalue);
            break;
        }
        case 0xF: { // MVN: Rd = ~Rs
            word newvalue = ~get_register(state, instr->rs);
            set_flags_nz(state, newvalue);
            set_register(state, instr->rd, newvalue);
            break;
        }
        default:
            logfatal("Unknown ALU operations opcode: %d", instr->opcode)
    }
}
