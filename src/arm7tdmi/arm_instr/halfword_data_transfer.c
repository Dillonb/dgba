#include "halfword_data_transfer.h"
#include "../sign_extension.h"

INLINE word rotate_right(word value, word amount) {
    amount &= 31u;
    return (value >> amount) | (value << (-amount & 31u));
}

INLINE void halfword_dt(arm7tdmi_t* state, bool p, bool u, bool w, bool l, byte rn, byte rd, word offset, bool s, bool h) {
    unimplemented(p == 0 && w == 1, "When p is 0, this bit must also always be 0!")
    if (p == 0) { w = true; } // Writeback always enabled when p == 0
    unimplemented(s == 0 && h == 0, "This is actually a SWP instruction, something went wrong and execution shouldn't be here")

    word addr = get_register(state, rn);
    if (p) {
        if (u) {
            addr += offset;
        } else {
            addr -= offset;
        }
    }

    if (l) {
        if (!s && h) { // LDRH: Load unsigned halfword (zero extended)
            half value = state->read_half(addr, ACCESS_UNKNOWN);
            set_register(state, rd, rotate_right(value, (addr & 1u) << 3u));
        } else if (s && !h) { //LDRSB: Load signed byte (sign extended)
            word value = state->read_byte(addr, ACCESS_UNKNOWN);
            set_register(state, rd, sign_extend_word(value, 8, 32));
        } else if (s && h) { // LDRSH: Load signed halfword (sign extended)
            word value;
            if (addr & 1) {
                value = state->read_byte(addr, ACCESS_UNKNOWN);
                value = sign_extend_word(value, 8, 32);
            } else {
                value = state->read_half(addr, ACCESS_UNKNOWN);
                value = sign_extend_word(value, 16, 32);
            }
            set_register(state, rd, value);
        }
    } else {
        logdebug("p: %d, u: %d, w: %d, l: %d, rn: %d, rd: %d, offset: %d, s: %d, h: %d", p, u, w, l, rn, rd, offset, s, h)
        if (!s && h) { // Store halfword
            unimplemented(rd == 15, "Special case for R15: PC")
            state->write_half(addr, get_register(state, rd), ACCESS_UNKNOWN);
        } else if (s && !h) { // Load doubleword
            logfatal("ldrd unimplemented (is this caught by LDR?)")
        } else if (s && h) { // Load halfword
            logfatal("strd unimplemented (is this caught by STR?)")
        }
    }

    if (w && (!l || rd != rn)) {
        if (!p) {
            if (u) {
                addr += offset;
            } else {
                addr -= offset;
            }
        }
        set_register(state, rn, addr);
    }
}

void halfword_dt_io(arm7tdmi_t* state, arminstr_t* arminstr) {
    halfword_dt_io_t instr = arminstr->parsed.HALFWORD_DT_IO;
    byte offset = instr.offset_low | (instr.offset_high << 4u);
    halfword_dt(state, instr.p, instr.u, instr.w, instr.l, instr.rn, instr.rd, offset, instr.s, instr.h);
}

void halfword_dt_ro(arm7tdmi_t* state, arminstr_t* arminstr) {
    halfword_dt_ro_t instr = arminstr->parsed.HALFWORD_DT_RO;
    halfword_dt(state, instr.p, instr.u, instr.w, instr.l, instr.rn, instr.rd, get_register(state, instr.rm), instr.s, instr.h);
}
