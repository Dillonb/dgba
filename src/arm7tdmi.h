#ifndef __ARM7TDMI_H__
#define __ARM7TDMI_H__

#include "util.h"

typedef struct arm7tdmi {
    // Connections to the bus
    byte (*read_byte)(uint32_t);
    uint16_t (*read16)(uint32_t);
    uint32_t (*read32)(uint32_t);
    void (*write_byte)(uint32_t, byte);
    void (*write16)(uint32_t, uint16_t);
    void (*write32)(uint32_t, uint32_t);

    // Registers
    // http://problemkaputt.de/gbatek.htm#armcpuflagsconditionfieldcond
    uint32_t r[13]; // General registers. Shared between modes.
    // !!!!! NOTE !!!!!
    // r8-r12 have separate values for FIQ mode, but that's only called by hardware debuggers
    // There is no way to trigger it from software, so they have been omitted.
    // These are technically r13, r14, and r15.
    // Don't put any other values between these and the r[13] array above.
    // This is so out of bounds access of that array will continue to work
    uint32_t sp;
    uint32_t lr;
    uint32_t pc;

    // Copies of registers for supervisor mode
    uint32_t r13_svc;
    uint32_t r14_svc;

    // Copies of registers for abort mode
    uint32_t r13_abt;
    uint32_t r14_abt;

    union {
        uint32_t raw;
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
    } cpsr;

    // Other state
    uint32_t pipeline[2];

} arm7tdmi_t;

arm7tdmi_t* init_arm7tdmi(byte (*read_byte)(uint32_t),
                          uint16_t (*read16)(uint32_t),
                          uint32_t (*read32)(uint32_t),
                          void (*write_byte)(uint32_t, byte),
                          void (*write16)(uint32_t, uint16_t),
                          void (*write32)(uint32_t, uint32_t));

int arm7tdmi_step(arm7tdmi_t* state);

#endif
