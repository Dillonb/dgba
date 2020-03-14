#include <stdbool.h>
#include "data_processing.h"
#include "../arm7tdmi.h"
#include "../../common/log.h"
#include "../shifts.h"

typedef union nonimmediate_flags {
    struct {
        unsigned:7; // common flags
        unsigned shift_amount:5;
    } shift_immediate;
    struct {
        unsigned:8; // common flags
        unsigned rs:4;
    } shift_register;
    // Common to both
    struct {
        unsigned rm:4;
        bool r:1; // 1: shift by register, 0, shift by immediate.
        shift_type_t shift_type:2;
    };
    unsigned raw:12;
} nonimmediate_flags_t;

// http://problemkaputt.de/gbatek.htm#armopcodesdataprocessingalu
void data_processing(arm7tdmi_t* state, data_processing_t* instr) {
    bool s = instr->s;
    byte rn = instr->rn;
    byte rd = instr->rd;


    if (instr->rd == 15) {
        s = false; // Don't update flags if we're dealing with the program counter
    }

    word operand2;

    word rndata = get_register(state, rn);

    if (instr->immediate) { // Operand2 comes from an immediate value
        operand2 = instr->operand2 & 0xFFu; // Last 8 bits of operand2 are the pre-shift value

        // first 4 bytes * 7 are the shift value
        // Only shift by 7 because we were going to multiply it by 2 anyway
        word shift = (instr->operand2 & 0xF00u) >> 7u;

        logdebug("Shift amount: %d", shift)
        logdebug("Operand2 before shift: %d", operand2)

        shift &= 31u;
        if (shift != 0) {
            operand2 = (operand2 >> shift) | (operand2 << (-shift & 31u));
            if (s) {
                state->cpsr.C = operand2 >> 31u;
            }
        }
    }
    else { // Operand2 comes from another register
        if (rn == 15) {
            rn += 4;
        }
        byte shift_amount;

        nonimmediate_flags_t flags;
        flags.raw = instr->operand2;

        //unimplemented(flags.rm == 15, "r15 is a special case")
        operand2 = get_register(state, flags.rm);
        if (flags.r && flags.rm == 15u) {
            operand2 += 4; // Special case for R15 when immediate
        }

        // Shift by register
        if (flags.r) {
            unimplemented(flags.shift_register.rs == 15, "r15 is a special case")
            shift_amount = get_register(state, flags.shift_register.rs) & 0xFFu; // Only lowest 8 bits used
            logdebug("Shift amount (r%d): 0x%02X", flags.shift_register.rs, shift_amount)
        }
        // Shift by immediate
        else {
            shift_amount = flags.shift_immediate.shift_amount;
            logdebug("Shift amount (immediate): 0x%02X", shift_amount)
        }



        logdebug("Operand before shift: 0x%08X", operand2)

        // Only pass cpsr if it should be updated
        status_register_t* cpsr = s?&state->cpsr:NULL;

        // Special case when shifting by immediate 0
        if (!flags.r && shift_amount == 0) {
            logdebug("----handling special case, immediate shift amount 0----")
            switch (flags.shift_type) {
                case LSL:
                    break; // Not affected.
                case LSR:
                    // Treat it as LSR#32
                    operand2 = arm_shift(cpsr, LSR, operand2, 32);
                    break;
                case ASR:
                    operand2 = arm_shift(cpsr, ASR, operand2, 32);
                    break;
                case ROR: {
                    word oldc = state->cpsr.C;
                    if (cpsr) {
                        cpsr->C = operand2 & 1u;
                        operand2 = (oldc << 31u) | (operand2 >> 1u);
                    }
                    break;
                }
            }
        } else {
            operand2 = arm_shift(cpsr, flags.shift_type, operand2, shift_amount);
        }
    }

    logdebug("Operand after shift: 0x%08X", operand2)

    logdebug("Doing data processing opcode %X", instr->opcode)

    switch(instr->opcode) {
        case 0x0: { // AND logical: Rd = Rn AND Op2
            word newvalue = rndata & operand2;
            if (s) { set_flags_nz(state, newvalue); }
            set_register(state, rd, newvalue);
            break;
        }
        case 0x1: { // XOR logical: Rd = Rn XOR Op2
            word newvalue = rndata ^ operand2;
            if (s) { set_flags_nz(state, newvalue); }
            set_register(state, rd, newvalue);
            break;
        }
        case 0x2: { // SUB: Rd = Rn-Op2
            word newvalue = rndata - operand2;
            if (s) {
                set_flags_nz(state, newvalue);
                set_flags_sub(state, rndata, operand2, newvalue);
            }
            set_register(state, rd, newvalue);
            break;
        }
        case 0x3: { // RSB (subtract reversed): Rd = Op2-Rn
            word newvalue = operand2 - rndata;
            if (s) {
                set_flags_nz(state, newvalue);
                set_flags_sub(state, operand2, rndata, newvalue);
            }
            set_register(state, rd, newvalue);
            break;
        }
        case 0x4: { // ADD: Rd = Rn+Op2
            word newvalue = rndata + operand2;
            if (s) {
                set_flags_nz(state, newvalue);
                set_flags_add(state, rndata, operand2);
            }
            set_register(state, rd, newvalue);
            break;
        }
        case 0x5: { // ADC: Rd = Rn+Op2+C
            uint64_t op2c = operand2 + state->cpsr.C;
            uint64_t newvalue = rndata + op2c;
            if (s) {
                set_flags_nz(state, newvalue);
                set_flags_add(state, op2c, rndata);
            }
            set_register(state, rd, newvalue);
            break;
        }
        case 0x6: { // SBC: Rd = Rn-Op2+C-1
            uint64_t tmp = operand2;
            tmp -= state->cpsr.C;
            tmp += 1;
            word newvalue = rndata - tmp;
            if (s) {
                set_flags_nz(state, newvalue);
                set_flags_sbc(state, rndata, operand2, tmp, newvalue);
            }
            set_register(state, rd, newvalue);
            break;
        }
        case 0x7: { // RSC: RD = Op2-Rn+C-1
            uint64_t tmp = rndata;
            tmp -= state->cpsr.C;
            tmp += 1;
            word newvalue = operand2 - tmp;
            if (s) {
                set_flags_nz(state, newvalue);
                set_flags_sbc(state, operand2, rndata, tmp, newvalue);
            }
            set_register(state, rd, newvalue);
            break;
        }
        case 0x8: { // TST: Void = Rn AND Op2
            unimplemented(!s, "BUG DETECTED: s flag must be set for opcodes 0x8-0xB")
            set_flags_nz(state, rndata & operand2);
            break;
        }
        case 0x9: { // TEQ: Void = Rn XOR Op2
            unimplemented(!s, "BUG DETECTED: s flag must be set for opcodes 0x8-0xB")
            set_flags_nz(state, rndata ^ operand2);
            break;
        }
        case 0xA: { // CMP: Void = Rn-Op2
            unimplemented(!s, "BUG DETECTED: s flag must be set for opcodes 0x8-0xB")
            word newvalue = rndata - operand2;
            set_flags_nz(state, newvalue);
            set_flags_sub(state, rndata, operand2, newvalue);
            break;
        }
        case 0xB: { // Void = Rn+Op2
            unimplemented(!s, "BUG DETECTED: s flag must be set for opcodes 0x8-0xB")
            word newvalue = rndata + operand2;
            set_flags_nz(state, newvalue);
            set_flags_add(state, rndata, operand2);
            break;
        }
        case 0xC: { // OR logical: Rd = Rn OR Op2
            word newvalue = rndata | operand2;
            if (s) { set_flags_nz(state, newvalue); }
            set_register(state, rd, newvalue);
            break;
        }
        case 0xD: { // MOV: Rd = Op2
            set_register(state, rd, operand2);
            if (s) { set_flags_nz(state, operand2); }
            break;
        }
        case 0xE: { // BIC: Rd = Rn AND NOT Op2
            word newvalue = rndata & (~operand2);
            if (s) { set_flags_nz(state, operand2); }
            set_register(state, rd, newvalue);
            break;
        }
        case 0xF: { // NOT: Rd = NOT Op2
            word newvalue = ~operand2;
            if (s) { set_flags_nz(state, operand2); }
            set_register(state, rd, newvalue);
            break;
        }
        default:
            logfatal("DATA_PROCESSING: unknown opcode: 0x%X", instr->opcode)
    }
}
