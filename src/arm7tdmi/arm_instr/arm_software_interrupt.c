#include "arm_software_interrupt.h"
#include "../../common/log.h"
#include "../software_interrupt.h"

void arm_software_interrupt(arm7tdmi_t* state, software_interrupt_t* instr) {
    software_interrupt(state, instr->comment >> 16);
}

