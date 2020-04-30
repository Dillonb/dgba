#ifndef __ARM7TDMI_H__
#define __ARM7TDMI_H__

#include <stdbool.h>

#include "../common/util.h"
#include "../common/log.h"

#define MODE_USER 0b10000
#define MODE_FIQ 0b10001
#define MODE_SUPERVISOR 0b10011
#define MODE_ABORT 0b10111
#define MODE_IRQ 0b10010
#define MODE_UNDEFINED 0b11011
#define MODE_SYSTEM 0b11111


#define REG_SP 13
#define REG_LR 14
#define REG_PC 15

typedef union status_register {
    word raw;
    struct {
        // Mode bits. "Current operating mode".
        unsigned mode:5;

        bool thumb:1;
        bool disable_fiq:1;
        bool disable_irq:1;

        unsigned RESERVED_DO_NOT_TOUCH:20;

        bool V:1;
        bool C:1;
        bool Z:1;
        bool N:1;
    };
} status_register_t;

typedef struct arm7tdmi {
    // Connections to the bus
    byte (*read_byte)(word);
    half (*read_half)(word);
    word (*read_word)(word);
    void (*write_byte)(word, byte);
    void (*write_half)(word, half);
    void (*write_word)(word, word);

    // Registers
    // http://problemkaputt.de/gbatek.htm#armcpuflagsconditionfieldcond
    word r[13]; // General registers. Shared between modes.

    // !!!!! NOTE !!!!!
    // r8-r12 have separate values for FIQ mode, but that's only called by hardware debuggers
    // There is no way to trigger it from software, other than setting the mode bits directly.
    word highreg_fiq[5];

    // These are technically r13, r14, and r15.
    word sp;
    word sp_fiq;
    word sp_svc;
    word sp_abt;
    word sp_irq;
    word sp_und;

    word lr;
    word lr_fiq;
    word lr_svc;
    word lr_abt;
    word lr_irq;
    word lr_und;

    word pc;

    // Copies of registers for supervisor mode
    word r13_svc;
    word r14_svc;

    // Copies of registers for abort mode
    word r13_abt;
    word r14_abt;

    status_register_t cpsr;
    status_register_t spsr;
    status_register_t spsr_fiq;
    status_register_t spsr_svc;
    status_register_t spsr_abt;
    status_register_t spsr_irq;
    status_register_t spsr_und;

    // Other state
    word pipeline[2];

    bool irq; // Should the CPU IRQ next chance it gets?
    bool halt; // Should the CPU do nothing (except interrupts?)

    word instr; // last instr the CPU executed

    int this_step_ticks;

    char disassembled[50];
} arm7tdmi_t;

arm7tdmi_t* init_arm7tdmi(byte (*read_byte)(word),
                          half (*read_half)(word),
                          word (*read_word)(word),
                          void (*write_byte)(word, byte),
                          void (*write_half)(word, half),
                          void (*write_word)(word, word));

int arm7tdmi_step(arm7tdmi_t* state);

void set_pc(arm7tdmi_t* state, word new_pc);

INLINE word get_sp(arm7tdmi_t* state) {
    switch (state->cpsr.mode) {
        case MODE_FIQ:
            return state->sp_fiq;
        case MODE_SUPERVISOR:
            return state->sp_svc;
        case MODE_ABORT:
            return state->sp_abt;
        case MODE_IRQ:
            return state->sp_irq;
        case MODE_UNDEFINED:
            return state->sp_und;
        default:
            return state->sp;
    }
}

INLINE word get_lr(arm7tdmi_t* state) {
    switch (state->cpsr.mode) {
        case MODE_FIQ:
            return state->lr_fiq;
        case MODE_SUPERVISOR:
            return state->lr_svc;
        case MODE_ABORT:
            return state->lr_abt;
        case MODE_IRQ:
            return state->lr_irq;
        case MODE_UNDEFINED:
            return state->lr_und;
        default:
            return state->lr;
    }
}

INLINE word get_register(arm7tdmi_t* state, word index) {
    word value = 0;
    if (state->cpsr.mode == MODE_FIQ && index >= 8 && index <= 12) {
        value = state->highreg_fiq[index - 8];
    } else if (index < 13) {
        value = state->r[index];
    } else if (index == 13) {
        value = get_sp(state);
    } else if (index == 14) {
        value = get_lr(state);
    } else if (index == 15) {
        value = state->pc;
    } else {
        logfatal("Attempted to read unknown register: r%d", index)
    }

    return value;
}

INLINE void set_sp(arm7tdmi_t* state, word newvalue) {
    switch (state->cpsr.mode) {
        case MODE_FIQ:
            state->sp_fiq = newvalue;
            break;
        case MODE_SUPERVISOR:
            state->sp_svc = newvalue;
            break;
        case MODE_ABORT:
            state->sp_abt = newvalue;
            break;
        case MODE_IRQ:
            state->sp_irq = newvalue;
            break;
        case MODE_UNDEFINED:
            state->sp_und = newvalue;
            break;
        default:
            state->sp = newvalue;
            break;
    }
}

INLINE void set_lr(arm7tdmi_t* state, word newvalue) {
    switch (state->cpsr.mode) {
        case MODE_FIQ:
            state->lr_fiq = newvalue;
            break;
        case MODE_SUPERVISOR:
            state->lr_svc = newvalue;
            break;
        case MODE_ABORT:
            state->lr_abt = newvalue;
            break;
        case MODE_IRQ:
            state->lr_irq = newvalue;
            break;
        case MODE_UNDEFINED:
            state->lr_und = newvalue;
            break;
        default:
            state->lr = newvalue;
            break;
    }
}

INLINE void set_register(arm7tdmi_t* state, word index, word newvalue) {
    logdebug("Set r%d to 0x%08X", index, newvalue)

    if (state->cpsr.mode == MODE_FIQ && index >= 8 && index <= 12) {
        state->highreg_fiq[index - 8] = newvalue;
    } else if (index < 13) {
        state->r[index] = newvalue;
    } else if (index == 13) {
        set_sp(state, newvalue);
    } else if (index == 14) {
        set_lr(state, newvalue);
    } else if (index == 15) {
        set_pc(state, newvalue);
    } else {
        logfatal("Attempted to write unknown register: r%d", index)
    }
}

// PSR, processor status register
status_register_t* get_psr(arm7tdmi_t* state);
void set_psr(arm7tdmi_t* state, word value);

// SPSR, saved processor status register
status_register_t* get_spsr(arm7tdmi_t* state);
void set_spsr(arm7tdmi_t* state, word value);

void set_flags_nz(arm7tdmi_t* state, word newvalue);
void set_flags_add(arm7tdmi_t* state, uint64_t op1, uint64_t op2);
void set_flags_sub(arm7tdmi_t* state, word op1, word op2, word result);
void set_flags_sbc(arm7tdmi_t* state, word op1, word op2, uint64_t tmp, word result);

void skip_bios(arm7tdmi_t* state);

#endif
