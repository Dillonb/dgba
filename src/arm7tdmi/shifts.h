#ifndef GBA_SHIFTS_H
#define GBA_SHIFTS_H

#include "../common/util.h"
#include "arm7tdmi.h"

word arm_lsl(status_register_t* cpsr, word data, word shift_amount);
word arm_lsr(status_register_t* cpsr, word data, word shift_amount);
word arm_asr(status_register_t* cpsr, word data, word shift_amount);
word arm_ror(status_register_t* cpsr, word data, word shift_amount);

word arm_shift(status_register_t* cpsr, byte type, word data, word shift_amount);

#endif //GBA_SHIFTS_H
