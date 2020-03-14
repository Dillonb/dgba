#ifndef __ARM7TDMI_H__
#define __ARM7TDMI_H__

#include <stdbool.h>

#include "../common/util.h"
#define MODE_USER 0b10000
#define MODE_FIQ 0b10001
#define MODE_SUPERVISOR 0b10011
#define MODE_ABORT 0b10111
#define MODE_IRQ 0b10010
#define MODE_UNDEFINED 0b11011

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
    // There is no way to trigger it from software, so they have been omitted.
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
    status_register_t spsr_fiq;
    status_register_t spsr_svc;
    status_register_t spsr_abt;
    status_register_t spsr_irq;
    status_register_t spsr_und;

    // Other state
    word pipeline[2];

    word instr; // last instr the CPU executed
} arm7tdmi_t;

arm7tdmi_t* init_arm7tdmi(byte (*read_byte)(word),
                          half (*read_half)(word),
                          word (*read_word)(word),
                          void (*write_byte)(word, byte),
                          void (*write_half)(word, half),
                          void (*write_word)(word, word));

int arm7tdmi_step(arm7tdmi_t* state);

word get_register(arm7tdmi_t* state, word index);
void set_register(arm7tdmi_t* state, word index, word newvalue);

word get_sp(arm7tdmi_t* state);

void set_pc(arm7tdmi_t* state, word new_pc);

// PSR, processor status register
status_register_t* get_psr(arm7tdmi_t* state);
void set_psr(arm7tdmi_t* state, word value);

// SPSR, saved processor status register
status_register_t* get_spsr(arm7tdmi_t* state);
void set_spsr(arm7tdmi_t* state, word value);

void set_flags_nz(arm7tdmi_t* state, word newvalue);
void set_flags_add(arm7tdmi_t* state, uint64_t op1, uint64_t op2);
void set_flags_sub(arm7tdmi_t* state, word op1, word op2);

void skip_bios(arm7tdmi_t* state);

#endif
