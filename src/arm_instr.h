#ifndef __ARM_INSTR_H__
#define __ARM_INSTR_H__

typedef enum arm_cond_t {
    EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL
} arm_cond;

typedef struct arminstr_t {
    union {
        uint32_t raw; // Used for loading data into this struct
        struct {
            arm_cond cond:4;
            union {
                unsigned raw:28;
            };
        } parsed;
    };
} arminstr;

#endif
