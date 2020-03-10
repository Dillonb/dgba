//
// Created by dillon on 3/10/20.
//

#include "immediate_operations.h"
#include "../../common/log.h"

void immediate_operations(arm7tdmi_t* state, byte opcode, byte rd, byte offset) {
    switch (opcode) {
        case 0: // MOV
            set_register(state, rd, offset);
            break;
        case 1: // CMP
            logfatal("Unimplmented opcode: CMP")
        case 2: // ADD
            logfatal("Unimplmented opcode: ADD")
        case 3: // SUB
            logfatal("Unimplmented opcode: SUB")
        default:
            logfatal("immediate_operations: opc: %d rd: %d offset: %d", opcode, rd, offset)
    }

    set_flags_nz(state, offset);
}
