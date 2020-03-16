#include "halfword_data_transfer.h"
#include "../../common/log.h"
#include "../sign_extension.h"

word rotate_right(word value, word amount) {
    amount &= 31u;
    return (value >> amount) | (value << (-amount & 31u));
}

void halfword_dt_io(arm7tdmi_t* state, bool p, bool u, bool w, bool l, byte rn, byte rd, word offset, bool s, bool h) {
    unimplemented(p == 0 && w == 1, "When p is 0, w must be 1!")
    unimplemented(s == 0 && h == 0, "This is actually a SWP instruction, something went wrong and execution shouldn't be here")
    unimplemented(w, "writeback")
    unimplemented(!u, "up flag not set")

    word addr = get_register(state, rn) + offset;

    if (l) {
        if (!s && h) { // LDRH: Load unsigned halfword (zero extended)
            half value = state->read_half(addr);
            set_register(state, rd, rotate_right(value, (addr & 1u) << 3u));
        } else if (s && !h) { //LDRSB: Load signed byte (sign extended)
            word value = state->read_byte(addr);
            set_register(state, rd, sign_extend_word(value, 8, 32));
        } else if (s && h) { // LDRSH: Load signed halfword (sign extended)
            word value;
            if (addr & 1) {
                value = state->read_byte(addr);
                value = sign_extend_word(value, 8, 32);
            } else {
                value = state->read_half(addr);
                value = sign_extend_word(value, 16, 32);
            }
            set_register(state, rd, value);
        }
    } else {
        logdebug("p: %d, u: %d, w: %d, l: %d, rn: %d, rd: %d, offset: %d, s: %d, h: %d", p, u, w, l, rn, rd, offset, s, h)
        if (!s && h) { // Store halfword
            unimplemented(rd == 15, "Special case for R15: PC")
            state->write_half(addr, get_register(state, rd));
        } else if (s && !h) { // Load doubleword
            logfatal("ldrd unimplemented (is this caught by LDR?)")
        } else if (s && h) { // Load halfword
            logfatal("strd unimplemented (is this caught by STR?)")
        }
    }

}

void halfword_dt_ro(arm7tdmi_t* state, bool p, bool u, bool w, bool l, byte rn, byte rd, bool s, bool h, byte rm) {
    halfword_dt_io(state, p, u, w, l, rn, rd, get_register(state, rm), s, h);
}
