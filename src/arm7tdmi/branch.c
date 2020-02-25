#include <stdbool.h>
#include "arm7tdmi.h"
#include "../common/log.h"

void branch(arm7tdmi_t* state, word offset, bool link) {
    bool thumb = offset & 1u;
    unimplemented(thumb, "THUMB mode")
    bool negative = (offset & 0b100000000000000000000000u) > 0;
    if (negative) {
        offset = ~offset + 1;
        logfatal("Encountered a branch with a negative offset. Make sure this is doing the right thing!")
    }
    loginfo("My offset is %d", offset << 2u)

    if (link) {
        logfatal("Branch-with-link isn't implemented yet.")
    }

    word newpc = (state->pc) + (offset << 2u);
    logdebug("Hold on to your hats, we're jumping to 0x%02X", newpc)
    set_pc(state, newpc);
}

