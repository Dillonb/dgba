//
// Created by dillon on 3/10/20.
//

#include "high_register_operations.h"
#include "../../common/log.h"

void high_register_operations(arm7tdmi_t* state, byte opcode, bool h1, bool h2, byte rshs, byte rdhd) {
    if (h1 == 0 && h2 == 0) {
        unimplemented(opcode == 0b00, "Invalid flag combination!")
        unimplemented(opcode == 0b01, "Invalid flag combination!")
        unimplemented(opcode == 0b10, "Invalid flag combination!")
    }
    switch (opcode) {
        case 0: // ADD
            logfatal("Unimplemented opcode: ADD")
        case 1: // CMP (only one that sets condition codes in this group)
            logfatal("Unimplemented opcode: CMP")
        case 2: { // MOV
            switch (h1 << 1 | h2) {
                case 0b10: { // MOV hd, rs
                    // TODO should this be a whole word or should we truncate to a half or a byte?
                    word source_data = get_register(state, rshs);
                    set_register(state, rdhd + 8, source_data);
                    break;
                }
                default:
                    logfatal("Unimplemented H flag combination: h1: %d h2: %d", h1, h2)
            }
        }
        case 3: // BX
            logfatal("Unimplemented opcode: BX")
    }
    logfatal("High register operations: opcode: %d h1: %d h2: %d rshs: %d rdhd: %d", opcode, h1, h2, rshs, rdhd)
}
