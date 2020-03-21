#include "alu_operations.h"
#include "../../common/log.h"

void alu_operations(arm7tdmi_t* state, alu_operations_t* instr) {
    switch (instr->opcode) {
        case 0x0: { // AND
            logfatal("AND")
        }
        case 0x1: { // EOR
            logfatal("EOR")
        }
        case 0x2: { // LSL
            logfatal("LSL")
        }
        case 0x3: { // LSR
            logfatal("LSR")
        }
        case 0x4: { // ASR
            logfatal("ASR")
        }
        case 0x5: { // ADC
            logfatal("ADC")
        }
        case 0x6: { // SBC
            logfatal("SBC")
        }
        case 0x7: { // ROR
            logfatal("ROR")
        }
        case 0x8: { // TST
            logfatal("TST")
        }
        case 0x9: { // NEG
            logfatal("NEG")
        }
        case 0xA: { // CMP
            logfatal("CMP")
        }
        case 0xB: { // CMN
            logfatal("CMN")
        }
        case 0xC: { // ORR
            word newvalue = get_register(state, instr->rd) | get_register(state, instr->rs);
            set_flags_nz(state, newvalue);
            set_register(state, instr->rd, newvalue);
            break;
        }
        case 0xD: { // MUL
            logfatal("MUL")
        }
        case 0xE: { // BIC
            logfatal("BIC")
        }
        case 0xF: { // MVN
            logfatal("MVN")
        }
        default:
            logfatal("Unknown ALU operations opcode: %d", instr->opcode)
    }
}
