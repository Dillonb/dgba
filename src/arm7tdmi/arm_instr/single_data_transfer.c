#include <stdbool.h>
#include "../arm7tdmi.h"
#include "../shifts.h"
#include "single_data_transfer.h"

typedef union immediate_as_offset_flags {
    struct {
        unsigned rm:4;
        bool reg_op:1; // Must be 0?
        unsigned shift_type:2;
        unsigned shift_amount:5;
        unsigned:4;
    };
    half raw;
} immediate_as_offset_flags_t;

// http://problemkaputt.de/gbatek.htm#armopcodesmemorysingledatatransferldrstrpld
void single_data_transfer(arm7tdmi_t* state, arminstr_t* arminstr) {
    //  When 1, Register shifted by Immediate as Offset
    unsigned int offset = arminstr->parsed.SINGLE_DATA_TRANSFER.offset;
    // dest if this is LDR, source if this is STR
    unsigned int rd = arminstr->parsed.SINGLE_DATA_TRANSFER.rd;
    unsigned int rn = arminstr->parsed.SINGLE_DATA_TRANSFER.rn;
    // 0 == str, 1 == ldr
    bool l = arminstr->parsed.SINGLE_DATA_TRANSFER.l;
    // different meanings depending on state of P (writeback)
    bool w = arminstr->parsed.SINGLE_DATA_TRANSFER.w;
    // (byte) when 0, transfer word, when 1, transfer byte
    bool b = arminstr->parsed.SINGLE_DATA_TRANSFER.b;
    // When 0, subtract offset from base, when 1, add to base
    bool up = arminstr->parsed.SINGLE_DATA_TRANSFER.u;
    // when 0, offset after transfer, when 1, before transfer.
    bool pre = arminstr->parsed.SINGLE_DATA_TRANSFER.p;
    //  When 0, Immediate as Offset
    bool immediate_offset_type = arminstr->parsed.SINGLE_DATA_TRANSFER.i;

    logdebug("l: %d w: %d b: %d u: %d p: %d i: %d", l, w, b, up, pre, immediate_offset_type)
    logdebug("rn: %d rd: %d, offset: 0x%03X", rn, rd, offset)
    if (!pre) {
        w = true;
    }
    if (w && rn == 15) {
        logfatal("Write-back must not be specified when Rn == R15")
    }

    int actual_offset;

    if (immediate_offset_type) {
        immediate_as_offset_flags_t flags;
        flags.raw = offset;
        unimplemented(flags.rm == 15, "Can't use r15 here!")

        unimplemented(flags.reg_op != 0, "The documentation told me this was always going to be 0")

        logdebug("Doing a shift type %d to the value of r%d by amount %d", flags.shift_type, flags.rm, flags.shift_amount)
        if (flags.shift_amount == 0) {
            actual_offset = arm_shift_special_zero_behavior(state, NULL, flags.shift_type, get_register(state, flags.rm));
        } else {
            actual_offset = arm_shift(NULL, flags.shift_type, get_register(state, flags.rm), flags.shift_amount);
        }
    } else {
        actual_offset = (int) offset;
    }

    if (!up) {
        actual_offset = -actual_offset;
    }

    word address = get_register(state, rn);

    if (pre) {
        address += actual_offset;
    }


    if (l) { // LDR
        word source;
        if (b) { // Read a byte
            logdebug("I'm gonna load r%d with a byte from 0x%08X", rd, address)
            source = state->read_byte(address, ACCESS_UNKNOWN);
        }
        else { // Read a word
            logdebug("I'm gonna load r%d with a word from 0x%08X", rd, address)
            source = state->read_word(address, ACCESS_UNKNOWN);
            source = arm_ror(NULL, source, (address & 3u) << 3);
        }
        logdebug("And that value is 0x%08X", source)
        set_register(state, rd, source);
    } else { // STR
        logdebug("I'm gonna save r%d to 0x%08X", rd, address)
        word rddata = get_register(state, rd);
        if (rd == 15) {
            rddata += 4;
        }
        if (b) {
            state->write_byte(address, rddata, ACCESS_UNKNOWN);
        } else {
            state->write_word(address, rddata, ACCESS_UNKNOWN);
        }
    }

    if (w && (!l || rd != rn)) {
        if (!pre) {
            address += actual_offset;
        }
        logdebug("Writing back 0x%08X to r%d", address, rn)
        set_register(state, rn, address);
    }
}
