#include <stdbool.h>
#include "arm7tdmi.h"
#include "../common/log.h"

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
    unimplemented(immediate_offset_type, "immediate_offset_type == 1 in single_data_transfer")
    unimplemented(l, "LDR")
    unimplemented(w, "w flag")
    unimplemented(!up, "down (subtract from base)")
    unimplemented(!pre, "post-transfer offset")
    unimplemented(rn == 15, "special case where rn == r15")

    word address = get_register(state, rn) + offset;

    logdebug("I'm gonna save r%d to 0x%02X", rd, address)
    state->write_word(address, get_register(state, rd));
}
