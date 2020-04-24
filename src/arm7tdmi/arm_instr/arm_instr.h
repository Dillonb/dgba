#ifndef __ARM_INSTR_H__
#define __ARM_INSTR_H__

#include <stdbool.h>
#include "../arm7tdmi.h"

typedef enum arm_cond_t {
    EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL, NV
} arm_cond_t;

typedef enum arm_instr_type {
    DATA_PROCESSING,
    STATUS_TRANSFER,
    MULTIPLY,
    MULTIPLY_LONG,
    SINGLE_DATA_SWAP,
    BRANCH_EXCHANGE,
    HALFWORD_DT_RO,
    HALFWORD_DT_IO,
    SINGLE_DATA_TRANSFER,
    UNDEFINED,
    BLOCK_DATA_TRANSFER,
    BRANCH,
    COPROCESSOR_DATA_TRANSFER,
    COPROCESSOR_DATA_OPERATION,
    COPROCESSOR_REGISTER_TRANSFER,
    SOFTWARE_INTERRUPT
} arm_instr_type_t;

typedef struct data_processing {
    unsigned operand2:12;
    unsigned rd:4;
    unsigned rn:4;
    bool s:1;
    unsigned opcode:4;
    bool immediate:1;
    unsigned:2;
    arm_cond_t cond:4;
} data_processing_t;

typedef struct multiply {
    unsigned rm:4;
    unsigned:4;
    unsigned rs:4;
    unsigned rn:4;
    unsigned rd:4;
    bool s:1;
    bool a:1;
    unsigned:6;
    arm_cond_t cond:4;
} multiply_t;

typedef struct multiply_long {
    unsigned rm:4;
    unsigned:4;
    unsigned rs:4;
    unsigned rdlo:4;
    unsigned rdhi:4;
    bool s:1;
    bool a:1;
    bool u:1;
    unsigned:5;
    arm_cond_t cond:4;
} multiply_long_t;

typedef struct single_data_swap {
    unsigned rm:4;
    unsigned:8;
    unsigned rd:4;
    unsigned rn:4;
    unsigned:2;
    bool b:1;
    unsigned:5;
    arm_cond_t cond:4;
} single_data_swap_t;

typedef struct branch_exchange {
    unsigned rn:4;
    unsigned opcode:4;
    unsigned:20;
    arm_cond_t cond:4;
} branch_exchange_t;

typedef struct halfword_dt_ro {
    unsigned rm:4;
    unsigned:1;
    bool h:1;
    bool s:1;
    unsigned:5;
    unsigned rd:4;
    unsigned rn:4;
    bool l:1;
    bool w:1;
    unsigned:1; // this is the immediate flag, always a 0 here, 1 in HALFWORD_DT_IO
    bool u:1;
    bool p:1;
    unsigned:3;
    arm_cond_t cond:4;
} halfword_dt_ro_t;

typedef struct halfword_dt_io {
    unsigned offset_low:4; // low 4 bits of 8 bit offset
    unsigned:1;
    bool h:1;
    bool s:1;
    unsigned:1;
    unsigned offset_high:4; // high 4 bits of 8 bit offset
    unsigned rd:4;
    unsigned rn:4;
    bool l:1;
    bool w:1;
    unsigned:1; // this is the immediate flag, always a 1 here, 0 in HALFWORD_DT_RO
    bool u:1;
    bool p:1;
    unsigned:3;
    arm_cond_t cond:4;
} halfword_dt_io_t;

typedef struct single_data_transfer {
    unsigned offset:12;
    unsigned rd:4;
    unsigned rn:4;
    bool l:1;
    bool w:1;
    bool b:1;
    bool u:1;
    bool p:1;
    bool i:1;
    unsigned:2;
    arm_cond_t cond:4;
} single_data_transfer_t ;

typedef struct undefined {
    unsigned TODO:32;
} undefined_t;

typedef struct block_data_transfer{
    unsigned rlist:16;
    unsigned rn:4;
    bool l:1;
    bool w:1;
    bool s:1;
    bool u:1;
    bool p:1;
    unsigned:3;
    arm_cond_t cond:4;
} block_data_transfer_t;

typedef struct branch {
    unsigned offset:24; // This value is actually signed, but needs to be this way because of how C works
    bool link:1;
    unsigned:3;
    arm_cond_t cond:4;
} branch_t;

typedef struct coprocessor_data_transfer {
    unsigned TODO:32;
} coprocessor_data_transfer_t;

typedef struct coprocessor_data_operation {
    unsigned TODO:32;
} coprocessor_data_operation_t;

typedef struct coprocessor_register_transfer {
    unsigned TODO:32;
} coprocessor_register_transfer_t;

typedef struct software_interrupt {
    unsigned comment:24;
    unsigned opcode:4;
    arm_cond_t cond:4;
} software_interrupt_t;

typedef union arminstr {
    unsigned raw:32; // Used for loading data into this struct
    struct {
        union { // Instruction, 28 bits
            struct {
                unsigned remaining:28;
                arm_cond_t cond:4;
            };
            data_processing_t DATA_PROCESSING;
            multiply_t MULTIPLY;
            multiply_long_t MULTIPLY_LONG;
            single_data_swap_t SINGLE_DATA_SWAP;
            branch_exchange_t BRANCH_EXCHANGE;
            halfword_dt_ro_t HALFWORD_DT_RO;
            halfword_dt_io_t HALFWORD_DT_IO;
            single_data_transfer_t SINGLE_DATA_TRANSFER;
            undefined_t UNDEFINED;
            block_data_transfer_t BLOCK_DATA_TRANSFER;
            branch_t BRANCH;
            coprocessor_data_transfer_t COPROCESSOR_DATA_TRANSFER;
            coprocessor_data_operation_t COPROCESSOR_DATA_OPERATION;
            coprocessor_register_transfer_t COPROCESSOR_REGISTER_TRANSFER;
            software_interrupt_t SOFTWARE_INTERRUPT;
        };
    } parsed;
} arminstr_t;

#define hash_arm_instr(instr) ((((instr) >> 16u) & 0xFF0u) | (((instr) >> 4u) & 0xFu))
void fill_arm_lut(void (*(*lut)[4096])(arm7tdmi_t*, arminstr_t*));
#endif
