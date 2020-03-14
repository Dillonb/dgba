#include <stdbool.h>
#include "../arm7tdmi.h"
#include "../../common/log.h"
#include "../shifts.h"

typedef union immediate_as_offset_flags {
    struct {
        unsigned rm:3; // Offset register
        unsigned:1; // Must be 0
        unsigned shift_type:2;
        unsigned shift_amount:5;
    };
    unsigned raw:11;
} immediate_as_offset_flags_t;

// http://problemkaputt.de/gbatek.htm#armopcodesmemorysingledatatransferldrstrpld
void single_data_transfer(arm7tdmi_t* state,
                          unsigned int offset,
                          unsigned int rd, // dest if this is LDR, source if this is STR
                          unsigned int rn,
                          bool l,   // 0 == str, 1 == ldr
                          bool w,   // different meanings depending on state of P (writeback)
                          bool b,   // (byte) when 0, transfer word, when 1, transfer byte
                          bool up,  // When 0, subtract offset from base, when 1, add to base
                          bool pre, // when 0, offset after transfer, when 1, before transfer.
                          bool immediate_offset_type) { //  When 0, Immediate as Offset
    //  When 1, Register shifted by Immediate as Offset
    logdebug("l: %d w: %d b: %d u: %d p: %d i: %d", l, w, b, up, pre, immediate_offset_type)
    logdebug("rn: %d rd: %d, offset: 0x%03X", rn, rd, offset)
    if (w && rn == 15) {
        logfatal("Write-back must not be specified when Rn == R15")
    }
    unimplemented(w, "w flag")
    unimplemented(!up, "down (subtract from base)")
    unimplemented(!pre, "post-transfer offset")

    word actual_offset;

    if (immediate_offset_type) {
        immediate_as_offset_flags_t flags;
        flags.raw = offset;
        unimplemented(flags.rm == 15, "Can't use r15 here!")

        actual_offset = arm_shift(NULL, flags.shift_type, get_register(state, flags.rm), flags.shift_amount);
    } else {
        actual_offset = offset;
    }

    word address = get_register(state, rn) + actual_offset;


    if (l) { // LDR
        word source;
        if (b) { // Read a byte
            logdebug("I'm gonna load r%d with a byte from 0x%08X", rd, address)
            source = state->read_byte(address);
        }
        else { // Read a word
            logdebug("I'm gonna load r%d with a word from 0x%08X", rd, address)
            source = state->read_word(address);
        }
        logdebug("And that value is 0x%08X", source)
        set_register(state, rd, source);
    } else { // STR
        unimplemented(rd == 15,
                      "When R15 is the source register (rd) of a register"
                      " store operation, the stored value will be the address of"
                      " the instruction plus 12. Make sure this is happening!")
        logdebug("I'm gonna save r%d to 0x%08X", rd, address)
        state->write_word(address, get_register(state, rd));
    }
}
