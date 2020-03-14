#include "immediate_operations.h"
#include "../../common/log.h"

void immediate_operations(arm7tdmi_t* state, immediate_operations_t* instr) {
    switch (instr->opcode) {
        case 0: // MOV
            set_register(state, instr->rd, instr->offset);
            break;
        case 1: // CMP
            logfatal("Unimplmented opcode: CMP")
        case 2: // ADD
            logfatal("Unimplmented opcode: ADD")
        case 3: // SUB
            logfatal("Unimplmented opcode: SUB")
        default:
            logfatal("immediate_operations: opc: %d rd: %d offset: %d", instr->opcode, instr->rd, instr->offset)
    }

    set_flags_nz(state, instr->offset);
}
