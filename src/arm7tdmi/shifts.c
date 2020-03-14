#include "shifts.h"
#include "../common/log.h"
#include "arm7tdmi.h"

word arm_lsr(status_register_t* cpsr, word data, word shift_amount) {
    unimplemented(shift_amount == 0, "shift amount 0 is a special case! see docs.")
    return data >> shift_amount;
}

word arm_lsl(status_register_t* cpsr, word data, word shift_amount) {
    if (shift_amount == 0) {
        return data;
    } else {
        word result;
        if (shift_amount < 32) {
            result = data << shift_amount;
            if (cpsr) {
                cpsr->C = (data << (shift_amount - 1u)) >> 31u;
            }
        }
        else {
            result = 0;
            if (cpsr) {
                // This has to be a special case since C doesn't like it when you << by >= the width of a type
                if (shift_amount == 32) {
                    cpsr->C = data & 1u;
                } else {
                    cpsr->C = 0;
                }
            }
        }
        logdebug("result: 0x%08X", result)
        return result;
    }

}

word arm_asr(status_register_t* cpsr, word data, word shift_amount) {
    unimplemented(shift_amount == 0, "shift amount 0 is a special case! see docs.")
    logfatal("ASR shift type unimplemented")
}

word arm_ror(status_register_t* cpsr, word data, word shift_amount) {
    unimplemented(shift_amount == 0, "shift amount 0 is a special case! see docs.")
    logfatal("ROR shift type unimplemented")
}

word arm_shift(status_register_t* cpsr, byte type, word data, word shift_amount) {
    switch (type) {
        case 0: // LSL
            return arm_lsl(cpsr, data, shift_amount);
        case 1: // LSR
            return arm_lsr(cpsr, data, shift_amount);
        case 2: // ASR
            return arm_asr(cpsr, data, shift_amount);
        case 3: // ROR
            return arm_ror(cpsr, data, shift_amount);
        default:
            logfatal("Unknown shift type: %d", type)
    }
}
