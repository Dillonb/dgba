#ifndef __ARM_INSTR_H__
#define __ARM_INSTR_H__

typedef enum arm_cond_t {
   EQ = 0,
   NE = 1,
   HS = 2,
   LO = 3,
   MI = 4,
   PL = 5,
   VS = 6,
   VC = 7,
   HI = 8,
   LS = 9,
   GE = 10,
   LT = 11,
   GT = 12,
   LE = 13,
   AL = 14,
   NV = 15,
} arm_cont;

typedef enum arm_instr_type_t {
    DPFSR,
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
} arm_instr_type;


typedef struct arminstr_t {
    union {
        uint32_t raw; // Used for loading data into this struct
        struct {
            arm_cond cond:4;
            union { // Instruction, 28 bits
                unsigned int raw:28;
                struct { // Data processing and FSR transfer
                    unsigned int identifier:3; // Equals 0b001 if instruction is this type
                    unsigned int raw:25; // TODO
                } DPFSR;
                struct {
                    unsigned int identifier:6; // Equals 0b000000 if instruction is this type
                    bool a:1;
                    bool s:1;
                    unsigned int rd:4;
                    unsigned int rn:4;
                    unsigned int rs:4;
                    unsigned int identifier2:4; // Equals 0b1001 if instruction is this type
                    unsigned int rm:4;
                } MULTIPLY;
                // TODO MULTIPLY_LONG
                // TODO SINGLE_DATA_SWAP
                // TODO BRANCH_EXCHANGE
                // TODO HALFWORD_DT_RO
                // TODO HALFWORD_DT_IO
                // TODO SINGLE_DATA_TRANSFER
                // TODO UNDEFINED
                // TODO BLOCK_DATA_TRANSFER
                struct {
                    unsigned int identifier:3; // Equals 0b101 if instruction is this type
                    bool l:1;
                    unsigned int offset:24;
                } BRANCH;
                // TODO COPROCESSOR_DATA_TRANSFER
                // TODO COPROCESSOR_DATA_OPERATION
                // TODO COPROCESSOR_REGISTER_TRANSFER
                // TODO SOFTWARE_INTERRUPT
            };
        } parsed;
    };
} arminstr;


bool is_multiply(arminstr* instr) {
    return instr->parsed.MULTIPLY.identifier == 0b001u && instr->parsed.MULTIPLY.identifier2 == 0b1001u;
}

bool is_branch(arminstr* instr) {
    return instr->parsed.BRANCH.identifier == 0b101u;
}

arm_instr_type get_instr_type(arminstr* instr) {
    if (is_multiply(instr)) {
        return MULTIPLY;
    } else if (is_branch(instr)) {
        return BRANCH;
    } else {
        logfatal("Unimplemented instruction type!")
    }
}

#endif
