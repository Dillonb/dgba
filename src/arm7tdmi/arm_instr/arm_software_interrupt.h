#include "../arm7tdmi.h"
#include "arm_instr.h"

#ifndef GBA_ARM_SOFTWARE_INTERRUPT_H
#define GBA_ARM_SOFTWARE_INTERRUPT_H
void arm_software_interrupt(arm7tdmi_t* state, software_interrupt_t* instr);
#endif //GBA_ARM_SOFTWARE_INTERRUPT_H
