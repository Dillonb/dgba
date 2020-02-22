#include <stdlib.h>
#include <stdbool.h>
#include "arm7tdmi.h"
#include "log.h"
#include "arm_instr.h"

arm7tdmi* init_arm7tdmi(byte (*read_byte)(uint32_t),
                        uint16_t (*read16)(uint32_t),
                        void (*write_byte)(uint32_t, byte),
                        void (*write16)(uint32_t, uint16_t)) {
    arm7tdmi* mem = malloc(sizeof(arm7tdmi));
    mem->pc = 0x08000000;
    mem->read_byte = read_byte;
    mem->read16 = read16;
    mem->write_byte = write_byte;
    mem->write16 = write16;

    return mem;
}

uint32_t read32(arm7tdmi* state, uint32_t addr) {
    uint32_t lower = state->read16(addr);
    uint32_t upper = state->read16(addr + 2);

    return (upper << 16u) | lower;
}

arminstr read32_instr(arm7tdmi* state, uint32_t addr) {
    arminstr instr;
    instr.raw = read32(state, addr);
    return instr;
}

bool check_cond(arminstr* instr) {
    switch (instr->parsed.cond) {
        case AL:
            return true;
        default:
            logfatal("Unimplemented COND: %d", instr->parsed.cond);
    }
}

void arm7tdmi_tick(arm7tdmi* state) {
    arminstr instr = read32_instr(state, state->pc);
    logdebug("read: 0x%04X", instr.raw);
    logdebug("cond: %d", instr.parsed.cond);
    logdebug("remaining: 0x%04X", instr.parsed.raw);
    if (!check_cond(&instr)) {
        // TODO 1 cycle
        return;
    }
    exit(0);
}
