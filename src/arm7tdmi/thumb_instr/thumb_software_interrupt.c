#include "thumb_software_interrupt.h"
#include "../../common/log.h"

void thumb_software_interrupt(arm7tdmi_t* state, thumb_software_interrupt_t* instr) {
    status_register_t cpsr = state->cpsr;
    state->cpsr.mode = MODE_SUPERVISOR;
    set_spsr(state, cpsr.raw);

    state->lr_svc = state->pc - 2;

    state->cpsr.thumb = 0;
    state->cpsr.disable_irq = 1;

    set_pc(state, 0x8); // SVC handler
}
