#include "arm_software_interrupt.h"
#include "../software_interrupt.h"

void arm_software_interrupt(arm7tdmi_t* state, arminstr_t * arminstr) {
    software_interrupt(state, arminstr->parsed.SOFTWARE_INTERRUPT.comment >> 16);
}

