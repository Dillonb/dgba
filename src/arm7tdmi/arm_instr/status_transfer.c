#include "status_transfer.h"
#include "../../common/log.h"

typedef union field_masks {
    struct {
        bool c:1; // Write to control field
        bool x:1; // Write to extension field
        bool s:1; // Write to status field
        bool f:1; // Write to flags field
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

typedef union msr_immediate_flags {
    struct {
        unsigned imm:8;
        unsigned shift:4;
    } parsed;
    unsigned raw:12;
} msr_immediate_flags_t;

// http://problemkaputt.de/gbatek.htm#armopcodespsrtransfermrsmsr
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

    if (opcode.msr) {
        field_masks_t field_masks;
        field_masks.raw = dt_rn; // field masks come from the "rn" field in data processing
        word mask = get_mask(&field_masks);

        word source_data;

        if (immediate) {
            msr_immediate_flags_t flags;
            flags.raw = dt_operand2;
            source_data = flags.parsed.imm;
            word shift = flags.parsed.shift * 2;

            shift &= 31u;
            source_data = (source_data >> shift) | (source_data << (-shift & 31u));
        }
        else {
            unsigned int source_register = dt_operand2 & 0b1111u;
            source_data = get_register(state, source_register);
        }

        logdebug("Source data: 0x%08X", source_data)
        source_data &= mask;
        logdebug("Mask: 0x%08X", mask)
        logdebug("Source data masked: 0x%08X", source_data)
        if (opcode.spsr) { // SPSR
            word spsr = get_spsr(state)->raw;
            spsr = (spsr & ~mask) | source_data;
            set_spsr(state, spsr);
        }
        else { // CPSR
            word psr = get_psr(state)->raw;
            psr = (psr & ~mask) | source_data;
            set_psr(state, psr);
        }
    }
    else {
        // MRS
        status_register_t* psr = opcode.spsr ? get_spsr(state) : get_psr(state);
        set_register(state, dt_rd, psr->raw);
    }

}

