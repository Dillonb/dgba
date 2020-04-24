#include <stdbool.h>
#include "thumb_instr.h"
#include "move_shifted_register.h"
#include "add_subtract.h"
#include "immediate_operations.h"
#include "alu_operations.h"
#include "high_register_operations.h"
#include "pc_relative_load.h"
#include "load_store.h"
#include "load_store_halfword.h"
#include "sp_relative_load_store.h"
#include "load_address.h"
#include "add_offset_to_stack_pointer.h"
#include "push_pop_registers.h"
#include "multiple_load_store.h"
#include "conditional_branch.h"
#include "thumb_software_interrupt.h"
#include "unconditional_branch.h"
#include "long_branch_link.h"
#include "../../common/log.h"

thumb_instr_type_t get_thumb_instr_type_hash(half hash) {
    if ((hash & 0b1111100000u) == 0b0001100000u) {
        return ADD_SUBTRACT;
    }
    if ((hash & 0b1110000000u) == 0b0000000000u) {
        return MOVE_SHIFTED_REGISTER;
    }
    if ((hash & 0b1110000000u) == 0b0010000000u) {
        return IMMEDIATE_OPERATIONS;
    }
    if ((hash & 0b1111110000u) == 0b0100000000u) {
        return ALU_OPERATIONS;
    }
    if ((hash & 0b1111110000u) == 0b0100010000u) {
        bool h1 = (hash >> 1u) & 1u;
        bool h2 = hash & 1u;
        byte opcode = (hash >> 2u) & 0b11u;

        if (opcode != 0b11 && h1 == 0 && h2 == 0) return THUMB_UNDEFINED;
        if (opcode == 0b11 && h1 == 1) return THUMB_UNDEFINED;

        return HIGH_REGISTER_OPERATIONS;
    }
    if ((hash & 0b1111100000u) == 0b0100100000u) return PC_RELATIVE_LOAD;
    if ((hash & 0b1111001000u) == 0b0101000000u) return LOAD_STORE_RO;
    if ((hash & 0b1111001000u) == 0b0101001000u) return LOAD_STORE_BYTE_HALFWORD;
    if ((hash & 0b1110000000u) == 0b0110000000u) return LOAD_STORE_IO;
    if ((hash & 0b1111000000u) == 0b1000000000u) return LOAD_STORE_HALFWORD;
    if ((hash & 0b1111000000u) == 0b1001000000u) return SP_RELATIVE_LOAD_STORE;
    if ((hash & 0b1111000000u) == 0b1010000000u) return LOAD_ADDRESS;
    if ((hash & 0b1111111100u) == 0b1011000000u) return ADD_OFFSET_TO_STACK_POINTER;
    if ((hash & 0b1111011000u) == 0b1011010000u) return PUSH_POP_REGISTERS;
    if ((hash & 0b1111000000u) == 0b1100000000u) return MULTIPLE_LOAD_STORE;
    if ((hash & 0b1111111100u) == 0b1101111100u) return THUMB_SOFTWARE_INTERRUPT;
    if ((hash & 0b1111000000u) == 0b1101000000u) {
        if (((hash >> 3u) & 0b111u) == 0b111u) {
            return THUMB_UNDEFINED;
        }

        return CONDITIONAL_BRANCH;
    }
    if ((hash & 0b1111100000u) == 0b1110000000u) return UNCONDITIONAL_BRANCH;
    if ((hash & 0b1111000000u) == 0b1111000000u) return LONG_BRANCH_LINK;

    return THUMB_UNDEFINED;
}

void unknown_instruction_thumb_undefined(arm7tdmi_t* state, thumbinstr_t* instr) {
    logfatal("Unimplemented THUMB mode instruction type: THUMB_UNDEFINED")
}

void fill_thm_lut(thminstr_handler_t (*lut)[1024]) {
    for (half i = 0; i < 1024; i++) {
        thumb_instr_type_t type = get_thumb_instr_type_hash(i);
        switch (type) {
            case MOVE_SHIFTED_REGISTER:
                (*lut)[i] = &move_shifted_register;
                break;
            case ADD_SUBTRACT:
                (*lut)[i] = &add_subtract;
                break;
            case IMMEDIATE_OPERATIONS:
                (*lut)[i] = &immediate_operations;
                break;
            case ALU_OPERATIONS:
                (*lut)[i] = &alu_operations;
                break;
            case HIGH_REGISTER_OPERATIONS:
                (*lut)[i] = &high_register_operations;
                break;
            case PC_RELATIVE_LOAD:
                (*lut)[i] = &pc_relative_load;
                break;
            case LOAD_STORE_RO:
                (*lut)[i] = &load_store_ro;
                break;
            case LOAD_STORE_BYTE_HALFWORD:
                (*lut)[i] = &load_store_byte_halfword;
                break;
            case LOAD_STORE_IO:
                (*lut)[i] = &load_store_io;
                break;
            case LOAD_STORE_HALFWORD:
                (*lut)[i] = &load_store_halfword;
                break;
            case SP_RELATIVE_LOAD_STORE:
                (*lut)[i] = &sp_relative_load_store;
                break;
            case LOAD_ADDRESS:
                (*lut)[i] = &load_address;
                break;
            case ADD_OFFSET_TO_STACK_POINTER:
                (*lut)[i] = &add_offset_to_stack_pointer;
                break;
            case PUSH_POP_REGISTERS:
                (*lut)[i] = &push_pop_registers;
                break;
            case MULTIPLE_LOAD_STORE:
                (*lut)[i] = &multiple_load_store;
                break;
            case CONDITIONAL_BRANCH:
                (*lut)[i] = &conditional_branch;
                break;
            case THUMB_SOFTWARE_INTERRUPT:
                (*lut)[i] = &thumb_software_interrupt;
                break;
            case UNCONDITIONAL_BRANCH:
                (*lut)[i] = &unconditional_branch;
                break;
            case LONG_BRANCH_LINK:
                (*lut)[i] = &long_branch_link;
                break;
            case THUMB_UNDEFINED:
                (*lut)[i] = &unknown_instruction_thumb_undefined;
                break;
            default:
                logfatal("Hit default case in arm_mode_step switch. This should never happen!")
        }
    }
}
