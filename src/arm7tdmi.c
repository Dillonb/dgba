#include <stdlib.h>
#include <stdbool.h>
#include "arm7tdmi.h"
#include "log.h"
#include "arm_instr.h"

uint32_t read32(arm7tdmi_t* state, uint32_t addr) {
    uint32_t lower = state->read16(addr);
    uint32_t upper = state->read16(addr + 2);

    return (upper << 16u) | lower;
}

void fill_pipe(arm7tdmi_t* state) {
    state->pipeline[0] = read32(state, state->pc);
    state->pc += 4;
    state->pipeline[1] = read32(state, state->pc);
    state->pc += 4;

    logdebug("Filling the instruction pipeline: 0x%08X = 0x%08X / 0x%08X = 0x%08X",
             state->pc - 8,
             state->pipeline[0],
             state->pc - 4,
             state->pipeline[1])
}

arm7tdmi_t* init_arm7tdmi(byte (*read_byte)(uint32_t),
                          uint16_t (*read16)(uint32_t),
                          void (*write_byte)(uint32_t, byte),
                          void (*write16)(uint32_t, uint16_t)) {
    arm7tdmi_t* state = malloc(sizeof(arm7tdmi_t));

    state->read_byte = read_byte;
    state->read16 = read16;
    state->write_byte = write_byte;
    state->write16 = write16;

    state->pc = 0x08000000;

    state->mode = ARM;

    fill_pipe(state);
    return state;
}

arminstr_t read32_instr(arm7tdmi_t* state, uint32_t addr) {
    arminstr_t instr;
    instr.raw = read32(state, addr);
    return instr;
}

bool check_cond(arminstr_t* instr) {
    switch (instr->parsed.cond) {
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
    state->pipeline[1] = read32(state, state->pc);

    return instr;
}

void branch(arm7tdmi_t* state, uint32_t offset, bool link) {
    bool thumb = offset & 1u;
    if (thumb) {
        logfatal("Attempted to activate THUMB mode, but this is not supported yet.");
    }
    bool negative = (offset & 0b100000000000000000000000u) > 0;
    if (negative) {
        offset = ~offset + 1;
        logfatal("Encountered a branch with a negative offset. Make sure this is doing the right thing!")
    }
    loginfo("My offset is %d", offset << 2u)

    if (link) {
        logfatal("Branch-with-link isn't implemented yet.")
    }

    uint32_t newpc = (state->pc) + (offset << 2u);
    logdebug("Hold on to your hats, we're jumping to 0x%02X", newpc)
    state->pc = newpc;
    fill_pipe(state);
}

int arm7tdmi_step(arm7tdmi_t* state) {
    this_step_ticks = 0;
    arminstr_t instr = next_instr(state);
    logdebug("read: 0x%04X", instr.raw)
    logdebug("cond: %d", instr.parsed.cond)
    if (check_cond(&instr)) {
        arm_instr_type_t type = get_instr_type(&instr);
        switch (type) {
            case DPFSR:
                logfatal("Unimplemented instruction type: DPFSR")
            case MULTIPLY:
                logfatal("Unimplemented instruction type: MULTIPLY")
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
                logfatal("Unimplemented instruction type: SINGLE_DATA_TRANSFER")
            case UNDEFINED:
                logfatal("Unimplemented instruction type: UNDEFINED")
            case BLOCK_DATA_TRANSFER:
                logfatal("Unimplemented instruction type: BLOCK_DATA_TRANSFER")
            case BRANCH:
                branch(state, instr.parsed.BRANCH.offset, instr.parsed.BRANCH.l);
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
    }
    else { // Cond told us not to execute this instruction
        tick(1);
    }
    return this_step_ticks;
}
