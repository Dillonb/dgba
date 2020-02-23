#ifndef __ARM7TDMI_H__
#define __ARM7TDMI_H__

#include "util.h"

typedef enum {
    ARM,
    THUMB
} cpu_mode;

typedef struct arm7tdmi {
    // Connections to the bus
    byte (*read_byte)(uint32_t);
    uint16_t (*read16)(uint32_t);
    void (*write_byte)(uint32_t, byte);
    void (*write16)(uint32_t, uint16_t);

    // Registers
    uint32_t pc;


    // Other state
    cpu_mode mode;
    uint32_t pipeline[2];
} arm7tdmi_t;

arm7tdmi_t* init_arm7tdmi(byte (*read_byte)(uint32_t),
                          uint16_t (*read16)(uint32_t),
                          void (*write_byte)(uint32_t, byte),
                          void (*write16)(uint32_t, uint16_t));

int arm7tdmi_step(arm7tdmi_t* state);

#endif
