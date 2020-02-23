#ifndef __ARM_INSTR_H__
#define __ARM_INSTR_H__

typedef enum arm_cond_t {
    EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL, NV
} arm_cond_t;

typedef enum arm_instr_type {
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
} arm_instr_type_t;


typedef union arminstr {
    unsigned raw:32; // Used for loading data into this struct
    struct {
        union { // Instruction, 28 bits
            struct {
                unsigned remaining:28;
                arm_cond_t cond:4;
            };
            // TODO DPFSR (Data processing and FSR transfer)
            struct {
                unsigned operand2:12;
                unsigned rd:4;
                unsigned rn:4;
                bool s:1;
                unsigned opcode:4;
                unsigned identifier:3; // Equals 0b001 if instruction is this type.
                arm_cond_t cond:4;
            } DPFSR;
            struct {
                unsigned rm:4;
                unsigned identifier2:4; // Equals 0b1001 if instruction is this type
                unsigned rs:4;
                unsigned rn:4;
                unsigned rd:4;
                bool s:1;
                bool a:1;
                unsigned identifier:6; // Equals 0b000000 if instruction is this type
                arm_cond_t cond:4;
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
                unsigned offset:24; // This value is actually signed, but needs to be this way because of how C works
                bool l:1;
                unsigned identifier:3; // Equals 0b101 if instruction is this type
                arm_cond_t cond:4;
            } BRANCH;
            // TODO COPROCESSOR_DATA_TRANSFER
            // TODO COPROCESSOR_DATA_OPERATION
            // TODO COPROCESSOR_REGISTER_TRANSFER
            // TODO SOFTWARE_INTERRUPT
        };
    } parsed;
} arminstr_t;

bool is_dpfsr(arminstr_t* instr) {
    return instr->parsed.DPFSR.identifier == 0b001u;
}

bool is_multiply(arminstr_t* instr) {
    return instr->parsed.MULTIPLY.identifier == 0b001u && instr->parsed.MULTIPLY.identifier2 == 0b1001u;
}

bool is_branch(arminstr_t* instr) {
    return instr->parsed.BRANCH.identifier == 0b101u;
}

arm_instr_type_t get_instr_type(arminstr_t* instr) {
    if (is_dpfsr(instr)) {
        return DPFSR;
    } else if (is_multiply(instr)) {
        return MULTIPLY;
    } else if (is_branch(instr)) {
        return BRANCH;
    } else {
        logfatal("Could not determine instruction type! 0x%04X", instr->raw)
    }
}

#endif
