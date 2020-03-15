#ifndef GBA_SHIFTS_H
#define GBA_SHIFTS_H

#include "../common/util.h"
#include "arm7tdmi.h"

typedef enum shift_type {
    LSL,
    LSR,
    ASR,
    ROR
} shift_type_t;

word arm_lsl(status_register_t* cpsr, word data, word shift_amount);
word arm_lsr(status_register_t* cpsr, word data, word shift_amount);
word arm_asr(status_register_t* cpsr, word data, word shift_amount);
word arm_ror(status_register_t* cpsr, word data, word shift_amount);

word arm_shift(status_register_t* cpsr, shift_type_t type, word data, word shift_amount);
word arm_shift_special_zero_behavior(arm7tdmi_t* state, status_register_t* cpsr, shift_type_t type, word data);

#endif //GBA_SHIFTS_H
