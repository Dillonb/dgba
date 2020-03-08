#ifndef __ARM_INSTR_H__
#define __ARM_INSTR_H__

typedef enum arm_cond_t {
    EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL, NV
} arm_cond_t;

typedef enum arm_instr_type {
    DATA_PROCESSING,
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

word instr_type_masks[] = {
        0b00001100000000000000000000000000, // DATA_PROCESSING
        0b00001111110000000000000010010000, // MULTIPLY
        0b00001111100000000000000010010000, // MULTIPLY_LONG
        0b00001111101100000000111111110000, // SINGLE_DATA_SWAP
        0b00001111111111111111111111110000, // BRANCH_EXCHANGE
        0b00001110010000000000111110010000, // HALFWORD_DT_RO
        0b00001110010000000000000010010000, // HALFWORD_DT_IO
        0b00001100000000000000000000000000, // SINGLE_DATA_TRANSFER
        0b00001110000000000000000000010000, // UNDEFINED
        0b00001110000000000000000000000000, // BLOCK_DATA_TRANSFER
        0b00001110000000000000000000000000, // BRANCH
        0b00001110000000000000000000000000, // CORPROCESSOR_DATA_TRANSFER
        0b00001111000000000000000000010000, // COPROCESSOR_DATA_OPERATION
        0b00001111000000000000000000010000, // COPROCESSOR_REGISTER_TRANSFER
        0b00001111000000000000000000000000, // SOFTWARE_INTERRUPT
};

word instr_type_identifiers[] = {
        0b00000000000000000000000000000000, // DATA_PROCESSING
        0b00000000000000000000000010010000, // MULTIPLY
        0b00000000100000000000000010010000, // MULTIPLY_LONG
        0b00000001000000000000000010010000, // SINGLE_DATA_SWAP
        0b00000001001011111111111100010000, // BRANCH_EXCHANGE
        0b00000000000000000000000010010000, // HALFWORD_DT_RO
        0b00000000010000000000000010010000, // HALFWORD_DT_IO
        0b00000100000000000000000000000000, // SINGLE_DATA_TRANSFER
        0b00000110000000000000000000010000, // UNDEFINED
        0b00001000000000000000000000000000, // BLOCK_DATA_TRANSFER
        0b00001010000000000000000000000000, // BRANCH
        0b00001100000000000000000000000000, // CORPROCESSOR_DATA_TRANSFER
        0b00001110000000000000000000000000, // COPROCESSOR_DATA_OPERATION
        0b00001110000000000000000000010000, // COPROCESSOR_REGISTER_TRANSFER
        0b00001111000000000000000000000000, // SOFTWARE_INTERRUPT
};


typedef union arminstr {
    unsigned raw:32; // Used for loading data into this struct
    struct {
        union { // Instruction, 28 bits
            struct {
                unsigned remaining:28;
                arm_cond_t cond:4;
            };
            struct {
                unsigned operand2:12;
                unsigned rd:4;
                unsigned rn:4;
                bool s:1;
                unsigned opcode:4;
                bool immediate:1;
                unsigned:2;
                arm_cond_t cond:4;
            } DATA_PROCESSING;
            struct {
                unsigned rm:4;
                unsigned:4;
                unsigned rs:4;
                unsigned rn:4;
                unsigned rd:4;
                bool s:1;
                bool a:1;
                unsigned:6;
                arm_cond_t cond:4;
            } MULTIPLY;
            // TODO MULTIPLY_LONG
            // TODO SINGLE_DATA_SWAP
            // TODO BRANCH_EXCHANGE
            // TODO HALFWORD_DT_RO
            // TODO HALFWORD_DT_IO
            struct {
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
            } SINGLE_DATA_TRANSFER;
            // TODO UNDEFINED
            // TODO BLOCK_DATA_TRANSFER
            struct {
                unsigned rlist:16;
                unsigned rn:4;
                bool l:1;
                bool w:1;
                bool s:1;
                bool u:1;
                bool p:1;
                unsigned:3;
                arm_cond_t cond:4;
            } BLOCK_DATA_TRANSFER;
            struct {
                unsigned offset:24; // This value is actually signed, but needs to be this way because of how C works
                bool l:1;
                unsigned:3;
                arm_cond_t cond:4;
            } BRANCH;
            // TODO COPROCESSOR_DATA_TRANSFER
            // TODO COPROCESSOR_DATA_OPERATION
            // TODO COPROCESSOR_REGISTER_TRANSFER
            // TODO SOFTWARE_INTERRUPT
        };
    } parsed;
} arminstr_t;

arm_instr_type_t get_instr_type(arminstr_t* instr) {
    for (arm_instr_type_t type = DATA_PROCESSING; type <= SOFTWARE_INTERRUPT; type++) {
        word masked = instr->raw & instr_type_masks[type];
        if (masked == instr_type_identifiers[type]) {
            return type;
        }
    }
    logfatal("Could not determine instruction type! 0x%04X", instr->raw)
}

#endif
