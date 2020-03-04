#ifndef __ARM7TDMI_H__
#define __ARM7TDMI_H__

#include "../common/util.h"

typedef union status_register {
    word raw;
    struct {
        bool N:1;
        bool Z:1;
        bool C:1;
        bool V:1;
        unsigned RESERVED_DO_NOT_TOUCH:20;
        bool disable_irq:1;
        bool disable_fiq:1;
        bool thumb:1;
        // Mode bits. "Current operating mode".
        // Does it make more sense to make this be a 5 bit value?
        bool m4:1;
        bool m3:1;
        bool m2:1;
        bool m1:1;
        bool m0:1;
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
    // Don't put any other values between these and the r[13] array above.
    // This is so out of bounds access of that array will continue to work
    word sp;
    word lr;
    word pc;

    // Copies of registers for supervisor mode
    word r13_svc;
    word r14_svc;

    // Copies of registers for abort mode
    word r13_abt;
    word r14_abt;

    status_register_t cpsr;

    // Other state
    word pipeline[2];

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

void set_pc(arm7tdmi_t* state, word new_pc);

// Gets the correct status register.
// For now, always returns CPSR, but when modes are implemented this will be mode-aware.
status_register_t* get_psr(arm7tdmi_t* state);

#endif
