#include <stdlib.h>
#include <stdbool.h>
#include "arm7tdmi.h"
#include "../common/log.h"
#include "arm_instr.h"
#include "data_processing.h"
#include "single_data_transfer.h"
#include "branch.h"

void fill_pipe(arm7tdmi_t* state) {
    state->pipeline[0] = state->read_word(state->pc);
    state->pc += 4;
    state->pipeline[1] = state->read_word(state->pc);
    state->pc += 4;

    logdebug("Filling the instruction pipeline: 0x%08X = 0x%08X / 0x%08X = 0x%08X",
             state->pc - 8,
             state->pipeline[0],
             state->pc - 4,
             state->pipeline[1])
}

void set_pc(arm7tdmi_t* state, word new_pc) {
    state->pc = new_pc;
    fill_pipe(state);
}

arm7tdmi_t* init_arm7tdmi(byte (*read_byte)(word),
                          half (*read_half)(word),
                          word (*read_word)(word),
                          void (*write_byte)(word, byte),
                          void (*write_half)(word, half),
                          void (*write_word)(word, word)) {
    arm7tdmi_t* state = malloc(sizeof(arm7tdmi_t));

    state->read_byte = read_byte;
    state->read_half = read_half;
    state->read_word = read_word;
    state->write_byte = write_byte;
    state->write_half = write_half;
    state->write_word = write_word;

    state->pc = 0x08000000;

    state->cpsr.raw = 0;

    fill_pipe(state);
    return state;
}

bool check_cond(arm7tdmi_t* state, arminstr_t* instr) {
    switch (instr->parsed.cond) {
        case EQ:
            return get_psr(state)->Z == 1;
        case AL:
            return true;
        default:
            logfatal("Unimplemented COND: %d", instr->parsed.cond);
    }
}

int this_step_ticks = 0;

void tick(int ticks) {
    this_step_ticks += ticks;
}

arminstr_t next_instr(arm7tdmi_t* state) {
    // TODO handle thumb mode

    arminstr_t instr;
    instr.raw = state->pipeline[0];
    state->pipeline[0] = state->pipeline[1];
    state->pipeline[1] = state->read_word(state->pc);

    return instr;
}

void set_register(arm7tdmi_t* state, word index, word newvalue) {
    if (index > 12) {
        logfatal("Tried to set a register > r12 - this has the possibility of being different depending on the mode, but that isn't implemented yet.")
    }
    logdebug("Set r%d to 0x%08X", index, newvalue)
    state->r[index] = newvalue;
}

word get_register(arm7tdmi_t* state, word index) {
    if (index > 12) {
        logfatal("Tried to get a register > r12 - this has the possibility of being different depending on the mode, but that isn't implemented yet.")
    }

    logdebug("Read the value of r%d: 0x%08X", index, state->r[index])
    return state->r[index];
}





int arm7tdmi_step(arm7tdmi_t* state) {
    this_step_ticks = 0;
    arminstr_t instr = next_instr(state);
    logwarn("adjusted pc: 0x%04X read: 0x%04X", state->pc - 8, instr.raw)
    logdebug("cond: %d", instr.parsed.cond)
    if (check_cond(state, &instr)) {
        arm_instr_type_t type = get_instr_type(&instr);
        switch (type) {
            case DATA_PROCESSING:
                data_processing(state,
                                instr.parsed.DATA_PROCESSING.operand2,
                                instr.parsed.DATA_PROCESSING.rd,
                                instr.parsed.DATA_PROCESSING.rn,
                                instr.parsed.DATA_PROCESSING.s,
                                instr.parsed.DATA_PROCESSING.immediate,
                                instr.parsed.DATA_PROCESSING.opcode);
                break;
            case MULTIPLY:
                unimplemented(1, "MULTIPLY instruction type")
            case MULTIPLY_LONG:
                logfatal("Unimplemented instruction type: MULTIPLY_LONG")
            case SINGLE_DATA_SWAP:
                logfatal("Unimplemented instruction type: SINGLE_DATA_SWAP")
            case BRANCH_EXCHANGE:
                logfatal("Unimplemented instruction type: BRANCH_EXCHANGE")
            case HALFWORD_DT_RO:
                logfatal("Unimplemented instruction type: HALFWORD_DT_RO")
            case HALFWORD_DT_IO:
                logfatal("Unimplemented instruction type: HALFWORD_DT_IO")
            case SINGLE_DATA_TRANSFER:
                single_data_transfer(state,
                                     instr.parsed.SINGLE_DATA_TRANSFER.offset,
                                     instr.parsed.SINGLE_DATA_TRANSFER.rd,
                                     instr.parsed.SINGLE_DATA_TRANSFER.rn,
                                     instr.parsed.SINGLE_DATA_TRANSFER.l,
                                     instr.parsed.SINGLE_DATA_TRANSFER.w,
                                     instr.parsed.SINGLE_DATA_TRANSFER.b,
                                     instr.parsed.SINGLE_DATA_TRANSFER.u,
                                     instr.parsed.SINGLE_DATA_TRANSFER.p,
                                     instr.parsed.SINGLE_DATA_TRANSFER.i);
                break;
            case UNDEFINED:
                logfatal("Unimplemented instruction type: UNDEFINED")
            case BLOCK_DATA_TRANSFER:
                logfatal("Unimplemented instruction type: BLOCK_DATA_TRANSFER")
            case BRANCH:
                branch(state, instr.parsed.BRANCH.offset, instr.parsed.BRANCH.l);
                state->pc -= 4; // This is to correct for the state->pc+=4 that happens after this switch
                break;
            case COPROCESSOR_DATA_TRANSFER:
                logfatal("Unimplemented instruction type: COPROCESSOR_DATA_TRANSFER")
            case COPROCESSOR_DATA_OPERATION:
                logfatal("Unimplemented instruction type: COPROCESSOR_DATA_OPERATION")
            case COPROCESSOR_REGISTER_TRANSFER:
                logfatal("Unimplemented instruction type: COPROCESSOR_REGISTER_TRANSFER")
            case SOFTWARE_INTERRUPT:
                logfatal("Unimplemented instruction type: SOFTWARE_INTERRUPT")
        }
        state->pc += 4;
    }
    else { // Cond told us not to execute this instruction
        tick(1);
    }
    return this_step_ticks;
}

// Gets the correct status register.
// For now, always returns CPSR, but when modes are implemented this will be mode-aware.
status_register_t* get_psr(arm7tdmi_t* state) {
    return &state->cpsr;
}

// Sets the correct status register.
// For now, always sets CPSR, but when modes are implemented this will be mode-aware.
void set_psr(arm7tdmi_t* state, word value) {
    state->cpsr.raw = value;
}
