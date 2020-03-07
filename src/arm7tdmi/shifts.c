//
// Created by dillon on 3/7/20.
//

#include "shifts.h"
#include "../common/log.h"

word arm_lsr(word data, word shift_amount) {
    return data >> shift_amount;
}

word arm_lsl(word data, word shift_amount) {
    return data << shift_amount;
}

word arm_asr(word data, word shift_amount) {
    logfatal("ASR shift type unimplemented")
}

word arm_ror(word data, word shift_amount) {
    logfatal("ROR shift type unimplemented")
}

word arm_shift(byte type, word data, word shift_amount) {
    switch (type) {
        case 0: // LSL
            return arm_lsl(data, shift_amount);
        case 1: // LSR
            return arm_lsr(data, shift_amount);
        case 2: // ASR
            return arm_asr(data, shift_amount);
        case 3: // ROR
            return arm_ror(data, shift_amount);
        default:
            logfatal("Unknown shift type: %d", type)
    }
}
