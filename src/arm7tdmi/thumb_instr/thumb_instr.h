#ifndef THUMB_INSTR_H
#define THUMB_INSTR_H
#include "../../common/util.h"

typedef enum thumb_instr_type {
    MOVE_SHIFTED_REGISTER,
    ADD_SUBTRACT,
    IMMEDIATE_OPERATIONS,
    ALU_OPERATIONS,
    HIGH_REGISTER_OPERATIONS,
    PC_RELATIVE_LOAD,
    LOAD_STORE_RO,
    LOAD_STORE_BYTE_HALFWORD, // Sign extended
    LOAD_STORE_IO,
    LOAD_STORE_HALFWORD,
    SP_RELATIVE_LOAD_STORE,
    LOAD_ADDRESS,
    ADD_OFFSET_TO_STACK_POINTER,
    PUSH_POP_REGISTERS,
    MULTIPLE_LOAD_STORE,
    CONDITIONAL_BRANCH,
    THUMB_SOFTWARE_INTERRUPT,
    UNCONDITIONAL_BRANCH,
    LONG_BRANCH_LINK,
    THUMB_UNDEFINED
} thumb_instr_type_t;

typedef struct immediate_operations {
    unsigned offset:8;
    unsigned rd:3;
    unsigned opcode:2;
    unsigned:3;
} immediate_operations_t;

typedef struct high_register_operations {
    unsigned rdhd:3;
    unsigned rshs:3;
    bool h2:1;
    bool h1:1;
    unsigned opcode:2;
    unsigned:6;
} high_register_operations_t;

typedef union thumbinstr {
    // TODO MOVE_SHIFTED_REGISTER
    // TODO ADD_SUBTRACT
    immediate_operations_t IMMEDIATE_OPERATIONS;
    // TODO ALU_OPERATIONS
    high_register_operations_t HIGH_REGISTER_OPERATIONS;
    half raw;
} thumbinstr_t;

thumb_instr_type_t get_thumb_instr_type(thumbinstr_t* instr);

#endif
