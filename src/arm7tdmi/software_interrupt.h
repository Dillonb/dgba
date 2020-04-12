#ifndef GBA_SOFTWARE_INTERRUPT_H
#define GBA_SOFTWARE_INTERRUPT_H

#include "../common/util.h"
#include "arm7tdmi.h"

void software_interrupt(arm7tdmi_t* state, byte comment);
#endif //GBA_SOFTWARE_INTERRUPT_H
