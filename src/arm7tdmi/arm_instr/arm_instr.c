#include "arm_instr.h"
#include "../../common/util.h"
#include "data_processing.h"
#include "../../common/log.h"
#include "status_transfer.h"
#include "multiply.h"
#include "single_data_swap.h"
#include "branch.h"
#include "halfword_data_transfer.h"
#include "single_data_transfer.h"
#include "block_data_transfer.h"
#include "arm_software_interrupt.h"

arm_instr_type_t get_arm_instr_type_hash(word hash) {
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

arm_instr_type_t get_arm_instr_type(arminstr_t* instr) {
    return get_arm_instr_type_hash(hash_arm_instr(instr->raw));
}

void unknown_instr_undefined(arm7tdmi_t* state, arminstr_t* instr) {
    logfatal("Unimplemented instruction type: UNDEFINED")
}

void unknown_instr_coprocessor_data_transfer(arm7tdmi_t* state, arminstr_t* instr) {
    logfatal("Unimplemented instruction type: COPROCESSOR_DATA_TRANSFER")
}

void unknown_instr_coprocessor_data_operation(arm7tdmi_t* state, arminstr_t* instr) {
    logfatal("Unimplemented instruction type: COPROCESSOR_DATA_OPERATION")
}

void unknown_instr_coprocessor_register_transfer(arm7tdmi_t* state, arminstr_t* instr) {
    logfatal("Unimplemented instruction type: COPROCESSOR_REGISTER_TRANSFER")
}

void fill_arm_lut(void (*(*lut)[4096])(arm7tdmi_t*, arminstr_t*)) {
    for (word i = 0; i < 4096; i++) {
        arm_instr_type_t type = get_arm_instr_type_hash(i);
        switch (type) {
            case DATA_PROCESSING:
                (*lut)[i] = &data_processing;
                break;
            case STATUS_TRANSFER:
                (*lut)[i] = &psr_transfer;
                break;
            case MULTIPLY:
                (*lut)[i] = &multiply;
                break;
            case MULTIPLY_LONG:
                (*lut)[i] = &multiply_long;
                break;
            case SINGLE_DATA_SWAP:
                (*lut)[i] = &single_data_swap;
                break;
            case BRANCH_EXCHANGE:
                (*lut)[i] = &branch_exchange;
                break;
            case HALFWORD_DT_RO:
                (*lut)[i] = &halfword_dt_ro;
                break;
            case HALFWORD_DT_IO:
                (*lut)[i] = &halfword_dt_io;
                break;
            case SINGLE_DATA_TRANSFER:
                (*lut)[i] = &single_data_transfer;
                break;
            case UNDEFINED:
                (*lut)[i] = &unknown_instr_undefined;
                break;
            case BLOCK_DATA_TRANSFER:
                (*lut)[i] = &block_data_transfer;
                break;
            case BRANCH:
                (*lut)[i] = &branch;
                break;
            case COPROCESSOR_DATA_TRANSFER:
                (*lut)[i] = &unknown_instr_coprocessor_data_transfer;
                break;
            case COPROCESSOR_DATA_OPERATION:
                (*lut)[i] = &unknown_instr_coprocessor_data_operation;
                break;
            case COPROCESSOR_REGISTER_TRANSFER:
                (*lut)[i] = &unknown_instr_coprocessor_register_transfer;
                break;
            case SOFTWARE_INTERRUPT:
                (*lut)[i] = &arm_software_interrupt;
                break;
            default:
                logfatal("Hit default case in fill_arm_lut switch. This should never happen!")
        }
    }
}
