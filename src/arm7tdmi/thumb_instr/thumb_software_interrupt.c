#include "thumb_software_interrupt.h"
#include "../software_interrupt.h"

void thumb_software_interrupt(arm7tdmi_t* state, thumbinstr_t* instr) {
    software_interrupt(state, instr->THUMB_SOFTWARE_INTERRUPT.value);
}
