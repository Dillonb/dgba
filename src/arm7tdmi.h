#ifndef __ARM7TDMI_H__
#define __ARM7TDMI_H__

#include "util.h"

typedef struct arm7tdmi_t {
    // Connections to the bus
    byte (*read_byte)(uint32_t);
    uint16_t (*read16)(uint32_t);
    void (*write_byte)(uint32_t, byte);
    void (*write16)(uint32_t, uint16_t);

    // Registers
    uint32_t pc;
} arm7tdmi;

arm7tdmi* init_arm7tdmi(byte (*read_byte)(uint32_t),
                        uint16_t (*read16)(uint32_t),
                        void (*write_byte)(uint32_t, byte),
                        void (*write16)(uint32_t, uint16_t));
void arm7tdmi_tick(arm7tdmi* state);

#endif
