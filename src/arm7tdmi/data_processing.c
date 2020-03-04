#include <stdbool.h>
#include "arm7tdmi.h"
#include "../common/log.h"

typedef union field_masks {
    struct {
        bool f:1; // Write to flags field
        bool s:1; // Write to status field
        bool x:1; // Write to extension field
        bool c:1; // Write to control field
    };
    unsigned raw:4;
} field_masks_t;

word get_mask(field_masks_t* masks) {
    word mask = 0;
    if (masks->f) mask |= 0xFF000000u;
    if (masks->s) mask |= 0x00FF0000u;
    if (masks->x) mask |= 0x0000FF00u;
    if (masks->c) mask |= 0x000000FFu;
    return mask;
}

void psr_transfer(arm7tdmi_t* state,
                  bool immediate,
                  unsigned int dt_opcode,
                  unsigned int dt_rn,
                  unsigned int dt_rd,
                  unsigned int dt_operand2) {
    union {
        struct {
            bool msr:1; // if 0, mrs, if 1, msr
            bool spsr:1; // if 0, cpsr, if 1, spsr_current mode
            unsigned:2;
        };
        unsigned raw:4;
    } opcode;
    opcode.raw = dt_opcode;

    unimplemented(!opcode.msr, "MRS mode unimplemented.")
    unimplemented(opcode.spsr, "SPSR not implemented")

    if (opcode.msr) {
        field_masks_t field_masks;
        field_masks.raw = dt_rn; // field masks come from the "rn" field in data processing
        word mask = get_mask(&field_masks);

        word source_data;

        if (immediate) {
            logfatal("Immediate not implemented")
        }
        else {
            unsigned int source_register = dt_operand2 & 0b1111u;
            // Debug
            printf("MSR CPSR_");
            if (field_masks.c) { printf("c"); }
            if (field_masks.x) { printf("x"); }
            if (field_masks.s) { printf("s"); }
            if (field_masks.f) { printf("f"); }
            printf(", r%d\n", source_register);
            // http://problemkaputt.de/gbatek.htm#armopcodespsrtransfermrsmsr

            source_data = get_register(state, source_register);
        }

        logdebug("Source data: 0x%08X", source_data)
        source_data &= mask;
        logdebug("Mask: 0x%08X", mask)
        logdebug("Source data masked: 0x%08X", source_data)
        logfatal("I know how to get the instruction, but not how to execute it.")
    }
    else {
        // MRS
        logfatal("Hello! This is an MRS instruction!")
    }

}

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
        return psr_transfer(state, immediate, opcode, rn, rd, immediate_operand2);
    }

    unimplemented(s, "updating condition codes flag in data processing")

    word operand2;

    if (immediate) { // Operand2 comes from an immediate value
        operand2 = immediate_operand2 & 0xFFu; // Last 8 bits of operand2 are the pre-shift value

        // first 4 bytes * 7 are the shift value
        // Only shift by 7 because we were going to multiply it by 2 anyway
        word shift = (immediate_operand2 & 0xF00u) >> 7u;

        operand2 &= 31u;
        operand2 = (operand2 >> shift) | (operand2 << (-shift & 31u));
    }
    else { // Operand2 comes from another register

        byte shift_amount;

        nonimmediate_flags_t flags;
        flags.raw = immediate_operand2;

        // Shift by register
        if (flags.r) {
            shift_amount = get_register(state, flags.shift_register.rs);
        }
        // Shift by immediate
        else {
            shift_amount = flags.shift_immediate.shift_amount;
        }

        logdebug("Shift amount: 0x%02X", shift_amount)

        switch (flags.shift_type) {
            default:
                logfatal("Unknown shift type: %d", flags.shift_type)
        }
    }


    switch(opcode) {
        case 0xC: // OR logical: Rd = Rn OR Op2
            set_register(state, rd, get_register(state, rn) | operand2);
            break;
        case 0xD: // MOV: Rd = Op2
            set_register(state, rd, operand2);
            break;
        default:
            logfatal("DATA_PROCESSING: unknown opcode: 0x%X", opcode)
    }
}
