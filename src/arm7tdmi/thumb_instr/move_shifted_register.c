#include "move_shifted_register.h"
#include "../../common/log.h"

void move_shifted_register(arm7tdmi_t* state, move_shifted_register_t* instr) {
    switch (instr->opcode) {
        case 0: { // LSL
            logfatal("LSL")
        }
        case 1: { // LSR
            logfatal("LSR")
        }
        case 2: { // ASR
            logfatal("ASR")
        }
        default:
            logfatal("Unknown MSR opcode: %d", instr->opcode)
    }
}
