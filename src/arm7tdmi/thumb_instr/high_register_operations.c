//
// Created by dillon on 3/10/20.
//

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
        case 0: // ADD
            logfatal("Unimplemented opcode: ADD")
        case 1: // CMP (only one that sets condition codes in this group)
            logfatal("Unimplemented opcode: CMP")
        case 2: { // MOV
            switch (h) {
                case 0b10: { // MOV hd, rs
                    // TODO should this be a whole word or should we truncate to a half or a byte?
                    word source_data = get_register(state, instr->rshs);
                    set_register(state, instr->rdhd + 8, source_data);
                    break;
                }
                default:
                    logfatal("Unimplemented H flag combination: h1: %d h2: %d", instr->h1, instr->h2)
            }
        }
        case 3: // BX
            logfatal("Unimplemented opcode: BX")
    }
    logfatal("High register operations: opcode: %d h1: %d h2: %d rshs: %d rdhd: %d", instr->opcode, instr->h1, instr->h2, instr->rshs, instr->rdhd)
}
