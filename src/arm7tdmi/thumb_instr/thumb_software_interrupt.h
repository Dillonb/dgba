#ifndef GBA_THUMB_SOFTWARE_INTERRUPT_H
#define GBA_THUMB_SOFTWARE_INTERRUPT_H

#include "../arm7tdmi.h"
#include "thumb_instr.h"

void thumb_software_interrupt(arm7tdmi_t* state, thumb_software_interrupt_t* instr);
#endif //GBA_THUMB_SOFTWARE_INTERRUPT_H
