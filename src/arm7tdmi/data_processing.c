#include <stdbool.h>
#include "arm7tdmi.h"
#include "../common/log.h"
#include "shifts.h"

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
        unsigned shift_type:2;
    };
    unsigned raw:12;
} nonimmediate_flags_t;

// http://problemkaputt.de/gbatek.htm#armopcodesdataprocessingalu
void data_processing(arm7tdmi_t* state,
                     word immediate_operand2,
                     word rd,
                     word rn,
                     bool s,
                     bool immediate,
                     word opcode) {
    // If it's one of these opcodes, and the s flag isn't set, this is actually a psr transfer op.
    if (!s && opcode >= 0x8 && opcode <= 0xb) { // TODO optimize with masks (opcode>>8) & 0b1100 == 0b1100 ?
        logfatal("This is actually a PSR transfer OP that got incorrectly detected as a data processing op!")
    }

    if (rd == 15) {
        s = false; // Don't update flags if we're dealing with the program counter
    }

    word operand2;

    word rndata = get_register(state, rn);

    if (immediate) { // Operand2 comes from an immediate value
        operand2 = immediate_operand2 & 0xFFu; // Last 8 bits of operand2 are the pre-shift value

        // first 4 bytes * 7 are the shift value
        // Only shift by 7 because we were going to multiply it by 2 anyway
        word shift = (immediate_operand2 & 0xF00u) >> 7u;

        logdebug("Shift amount: %d", shift)
        logdebug("Operand2 before shift: %d", operand2)

        shift &= 31u;
        operand2 = (operand2 >> shift) | (operand2 << (-shift & 31u));
    }
    else { // Operand2 comes from another register
        if (rn == 15) {
            rn += 4;
        }
        byte shift_amount;

        nonimmediate_flags_t flags;
        flags.raw = immediate_operand2;

        unimplemented(flags.rm == 15, "r15 is a special case")
        operand2 = get_register(state, flags.rm);
        if (flags.rm == 15u) {
            operand2 += 4; // Special case for R15 when immediate
        }

        // Shift by register
        if (flags.r) {
            unimplemented(flags.shift_register.rs == 15, "r15 is a special case")
            shift_amount = get_register(state, flags.shift_register.rs) & 0xFFu; // Only lowest 8 bits used
            unimplemented(shift_amount == 0, "shift amount 0 is a special case! see docs.")
        }
        // Shift by immediate
        else {
            shift_amount = flags.shift_immediate.shift_amount;
        }

        logdebug("Shift amount: 0x%02X", shift_amount)

        // Needed when s == true - set condition codes on status register
        // status_register_t* psr = get_psr(state);

        logdebug("Operand before shift: 0x%08X", operand2)

        operand2 = arm_shift(flags.shift_type, operand2, shift_amount);
    }

    logdebug("Operand after shift: 0x%08X", operand2)


    switch(opcode) {
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
                set_flags_sub(state, rndata, operand2);
            }
            set_register(state, rd, newvalue);
            break;
        }
        case 0x3: { // RSB (subtract reversed): Rd = Op2-Rn
            word newvalue = operand2 - rndata;
            if (s) {
                set_flags_nz(state, newvalue);
                set_flags_sub(state, operand2, rndata);
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
        case 0xA: { // CMP: Void = Rn-Op2
            unimplemented(!s, "BUG DETECTED: s flag must be set for opcodes 0x8-0xB")
            set_flags_nz(state, rndata - operand2);
            set_flags_sub(state, rndata, operand2);
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
            logfatal("DATA_PROCESSING: unknown opcode: 0x%X", opcode)
    }
}
