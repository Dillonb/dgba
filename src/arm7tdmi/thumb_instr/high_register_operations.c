#include "high_register_operations.h"

void high_register_operations(arm7tdmi_t* state, thumbinstr_t* thminstr) {
    high_register_operations_t* instr = &thminstr->HIGH_REGISTER_OPERATIONS;
    if (instr->h1 == 0 && instr->h2 == 0) {
        unimplemented(instr->opcode == 0b00, "Invalid flag combination!")
        unimplemented(instr->opcode == 0b01, "Invalid flag combination!")
        unimplemented(instr->opcode == 0b10, "Invalid flag combination!")
    }
    switch (instr->opcode) {
        case 0b00: { // ADD
            int adj_rd = instr->h1 ? instr->rdhd + 8 : instr->rdhd;
            int adj_rs = instr->h2 ? instr->rshs + 8 : instr->rshs;
            word rsdata = get_register(state, adj_rs);
            word rddata = get_register(state, adj_rd);
            word result = rsdata + rddata;
            if (adj_rd == REG_PC) {
                result |= 1; // Set thumb mode bit
            }
            set_register(state, adj_rd, result);
            break;
        }
        case 0b01: { // CMP (only one that sets condition codes in this group)
            int adj_rd = instr->h1 ? instr->rdhd + 8 : instr->rdhd;
            int adj_rs = instr->h2 ? instr->rshs + 8 : instr->rshs;
            word rsdata = get_register(state, adj_rs);
            word rddata = get_register(state, adj_rd);
            word result = rddata - rsdata;
            set_flags_sub(state, rddata, rsdata, result);
            set_flags_nz(state, result);
            break;
        }
        case 0b10: { // MOV
            int adj_rd = instr->h1 ? instr->rdhd + 8 : instr->rdhd;
            int adj_rs = instr->h2 ? instr->rshs + 8 : instr->rshs;
            word source_data = get_register(state, adj_rs);
            if (adj_rd == REG_PC) {
                source_data |= 1; // Set thumb mode bit
            }
            set_register(state, adj_rd, source_data);
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
            loginfo("Hold on to your (high register operations) hats, we're jumping to 0x%02X", newpc)
            if (!thumb) logdebug("REALLY hang on, we're exiting THUMB mode!")
            set_pc(state, newpc);
            break;
        }
    }
}
