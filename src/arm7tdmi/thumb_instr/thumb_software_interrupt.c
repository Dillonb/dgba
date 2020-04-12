#include "thumb_software_interrupt.h"
#include "../software_interrupt.h"

void thumb_software_interrupt(arm7tdmi_t* state, thumb_software_interrupt_t* instr) {
    software_interrupt(state, instr->value);
}
