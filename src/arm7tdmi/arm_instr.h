#ifndef __ARM_INSTR_H__
#define __ARM_INSTR_H__

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
            struct {
                unsigned rn:4;
                unsigned opcode:4;
                unsigned:20;
                arm_cond_t cond:4;
            } BRANCH_EXCHANGE;
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
    word hash = instr->raw;
    hash = ((hash >> 16u) & 0xFF0u) | ((hash >> 4u) & 0xFu);

    if ((hash & 0b111100000000u) == 0b111100000000u) {
        return SOFTWARE_INTERRUPT;
    }
    if ((hash & 0b111000000000u) == 0b110000000000u) {
        return COPROCESSOR_DATA_TRANSFER;
    }
    if ((hash & 0b111100000001u) == 0b111000000000u) {
        return COPROCESSOR_DATA_OPERATION;
    }
    if ((hash & 0b111100000001u) == 0b111000000001u) {
        return COPROCESSOR_REGISTER_TRANSFER;
    }
    if ((hash & 0b111000000000u) == 0b101000000000u) {
        return BRANCH;
    }
    if ((hash & 0b111000000000u) == 0b100000000000u) {
        return BLOCK_DATA_TRANSFER;
    }
    if ((hash & 0b111000000001u) == 0b011000000001u) {
        return UNDEFINED;
    }
    if ((hash & 0b110000000000u) == 0b010000000000u) {
        return SINGLE_DATA_TRANSFER;
    }
    if ((hash & 0b111111111111u) == 0b000100100001u) {
        return BRANCH_EXCHANGE;
    }
    if ((hash & 0b111111001111u) == 0b000000001001u) {
        return MULTIPLY;
    }
    if ((hash & 0b111110001111u) == 0b000010001001u) {
        return MULTIPLY_LONG;
    }
    if ((hash & 0b111110111111u) == 0b000100001001u) {
        return SINGLE_DATA_SWAP;
    }

    if ((hash & 0b111000001001u) == 0b000000001001u) {
        byte opcode = (hash >> 1u) & 0b11u;

        if (opcode == 0b00){
            return UNDEFINED;
        }
        else {
            logfatal("Half signed data transfer.")
        }
    }
    if ((hash & 0b110110010000u) == 0b000100000000u) {
        return STATUS_TRANSFER;
    }

    if ((hash & 0b110000000000u) == 0b000000000000u) {
        byte flags = (hash >> 4u) & 1u;
        byte opcode = (hash >> 5u) & 0b1111u;

        if ((opcode >> 2u) == 0b10 && !flags) {
            return UNDEFINED;
        }
        else {
            return DATA_PROCESSING;
        }
    }
    return UNDEFINED;
}

#endif
