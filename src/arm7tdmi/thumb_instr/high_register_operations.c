#include "high_register_operations.h"
#include "../../common/log.h"

void high_register_operations(arm7tdmi_t* state, high_register_operations_t* instr) {
    byte h = instr->h1 << 1 | instr->h2;
    if (h == 0) {
        unimplemented(instr->opcode == 0b00, "Invalid flag combination!")
        unimplemented(instr->opcode == 0b01, "Invalid flag combination!")
        unimplemented(instr->opcode == 0b10, "Invalid flag combination!")
    }
    switch (instr->opcode) {
        case 0b00: // ADD
            logfatal("Unimplemented opcode: ADD")
        case 0b01: // CMP (only one that sets condition codes in this group)
            logfatal("Unimplemented opcode: CMP")
        case 0b10: { // MOV
            if (h == 0b10) { // MOV hd, rs
                    // TODO should this be a whole word or should we truncate to a half or a byte?
                    unimplemented(instr->rdhd + 8 == 15, "R15 == PC is a special case")
                    word source_data = get_register(state, instr->rshs);
                    set_register(state, instr->rdhd + 8, source_data);
            } else {
                logfatal("Unimplemented H flag combination: h1: %d h2: %d", instr->h1, instr->h2)
            }
            break;
        }
        case 0b11: { // BX
            unimplemented(instr->h1 == 1, "INVALID setting for h1")
            word newpc;
            if (instr->h2) {
                newpc = get_register(state, instr->rshs + 8);
            } else {
                newpc = get_register(state, instr->rshs);
            }

            bool thumb = newpc & 1u;
            loginfo("Hold on to your hats, we're jumping to 0x%02X", newpc)
            if (!thumb) logdebug("REALLY hang on, we're exiting THUMB mode!")
            set_pc(state, newpc);
            break;
        }
    }
}
