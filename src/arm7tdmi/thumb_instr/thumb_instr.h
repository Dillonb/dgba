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

typedef union thumbinstr {
    half raw;
} thumbinstr_t;

thumb_instr_type_t get_thumb_instr_type(thumbinstr_t* instr) {
    half hash = instr->raw >> 6u;

    if ((hash & 0b1110000000u) == 0b0000000000u) {
        return MOVE_SHIFTED_REGISTER;
    }
    if ((hash & 0b1111100000u) == 0b0001100000u) {
        return ADD_SUBTRACT;
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
