//
// Created by dillon on 3/8/20.
//

#include "halfword_data_transfer.h"
#include "../common/log.h"

word rotate_right(word value, word amount) {
    amount &= 31u;
    return (value >> amount) | (value << (-amount & 31u));
}

void halfword_dt_io(arm7tdmi_t* state, bool p, bool u, bool w, bool l, byte rn, byte rd, word offset, bool s, bool h) {
    unimplemented(p == 0 && w == 1, "When p is 0, w must be 1!")
    unimplemented(s == 0 && h == 0, "This is actually a SWP instruction, something went wrong and execution shouldn't be here")
    unimplemented(w, "writeback")
    unimplemented(!u, "up flag not set")

    word source_addr = get_register(state, rn) + offset;

    if (l) {
        if (!s && h) { // Load unsigned halfword (zero extended)
            half value = state->read_half(source_addr);
            set_register(state, rd, rotate_right(value, (source_addr & 1u) << 3u));
            logdebug("ldrh")
        } else if (s && !h) { // Load signed byte (sign extended)
            logfatal("ldrsb unimplemented")
        } else if (s && h) { // Load signed halfword (sign extended)
            logfatal("ldrsh unimplemented")
        }
    } else {
        unimplemented(!l, "!l, store case not implemented yet.")
    }

    logdebug("p: %d, u: %d, w: %d, l: %d, rn: %d, rd: %d, offset: %d, s: %d, h: %d", p, u, w, l, rn, rd, offset, s, h)
}

void halfword_dt_ro(arm7tdmi_t* state, bool p, bool u, bool w, bool l, byte rn, byte rd, bool s, bool h, byte rm) {
    halfword_dt_io(state, p, u, w, l, rn, rd, get_register(state, rm), s, h);
}
