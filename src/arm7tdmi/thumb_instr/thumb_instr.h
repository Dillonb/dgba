#ifndef THUMB_INSTR_H
#define THUMB_INSTR_H
#include <stdbool.h>
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

typedef struct move_shifted_register {
    unsigned rd:3;
    unsigned rs:3;
    unsigned offset:5;
    unsigned opcode:2;
    unsigned:3;
} move_shifted_register_t;

typedef struct add_subtract {
    unsigned rd:3;
    unsigned rs:3;
    unsigned rn_or_offset:3;
    bool op:1;
    bool i:1;
    unsigned:5;
} add_subtract_t;

typedef struct immediate_operations {
    unsigned offset:8;
    unsigned rd:3;
    unsigned opcode:2;
    unsigned:3;
} immediate_operations_t;

typedef struct alu_operations {
    unsigned rd:3;
    unsigned rs:3;
    unsigned opcode:4;
    unsigned:6;
} alu_operations_t;

typedef struct high_register_operations {
    unsigned rdhd:3;
    unsigned rshs:3;
    bool h2:1;
    bool h1:1;
    unsigned opcode:2;
    unsigned:6;
} high_register_operations_t;

typedef struct pc_relative_load {
    unsigned word8:8;
    unsigned rd:3;
    unsigned:5;
} pc_relative_load_t;

typedef struct load_address {
    unsigned word8:8;
    unsigned rd:3;
    bool sp:1;
    unsigned:4;
} load_address_t;

typedef union thumbinstr {
    move_shifted_register_t MOVE_SHIFTED_REGISTER;
    add_subtract_t ADD_SUBTRACT;
    immediate_operations_t IMMEDIATE_OPERATIONS;
    alu_operations_t ALU_OPERATIONS;
    high_register_operations_t HIGH_REGISTER_OPERATIONS;
    pc_relative_load_t PC_RELATIVE_LOAD;
    load_address_t LOAD_ADDRESS;
    half raw;
} thumbinstr_t;

thumb_instr_type_t get_thumb_instr_type(thumbinstr_t* instr);

#endif
