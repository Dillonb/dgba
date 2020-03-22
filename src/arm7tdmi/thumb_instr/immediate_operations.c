#include "immediate_operations.h"
#include "../../common/log.h"

void immediate_operations(arm7tdmi_t* state, immediate_operations_t* instr) {
    switch (instr->opcode) {
        case 0: // MOV
            set_register(state, instr->rd, instr->offset);
            set_flags_nz(state, instr->offset);
            break;
        case 1: { // CMP
            word rddata = get_register(state, instr->rd);
            word result = rddata - instr->offset;
            set_flags_nz(state, result);
            set_flags_sub(state, rddata, instr->offset, result);
            break;
        }
        case 2: { // ADD
            word rddata = get_register(state, instr->rd);
            word result = rddata + instr->offset;
            set_flags_add(state, rddata, instr->offset);
            set_flags_nz(state, result);
            set_register(state, instr->rd, result);
            break;
        }
        case 3: { // SUB
            word rddata = get_register(state, instr->rd);
            word result = rddata - instr->offset;
            set_flags_sub(state, rddata, result, instr->offset);
            set_flags_nz(state, result);
            set_register(state, instr->rd, result);
            break;
        }
        default:
            logfatal("immediate_operations: opc: %d rd: %d offset: %d", instr->opcode, instr->rd, instr->offset)
    }
}
