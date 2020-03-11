#include "arm_instr.h"
#include "../../common/util.h"

arm_instr_type_t get_arm_instr_type(arminstr_t* instr) {
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
        else if (hash & 0x40u) {
            return HALFWORD_DT_IO;
        }
        else {
            return HALFWORD_DT_RO;
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
