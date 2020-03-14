#ifndef GBA_SHIFTS_H
#define GBA_SHIFTS_H

#include "../common/util.h"

word arm_lsl(word data, word shift_amount);
word arm_lsr(word data, word shift_amount);
word arm_asr(word data, word shift_amount);
word arm_ror(word data, word shift_amount);

word arm_shift(byte type, word data, word shift_amount);

#endif //GBA_SHIFTS_H
