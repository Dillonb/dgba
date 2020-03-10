#include <stdbool.h>

#include "../arm7tdmi.h"

#ifndef GBA_HALFWORD_DATA_TRANSFER_H
#define GBA_HALFWORD_DATA_TRANSFER_H

void halfword_dt_io(arm7tdmi_t* state, bool p, bool u, bool w, bool l, byte rn, byte rd, word offset, bool s, bool h);
void halfword_dt_ro(arm7tdmi_t* state, bool p, bool u, bool w, bool l, byte rn, byte rd, bool s, bool h, byte rm);

#endif //GBA_HALFWORD_DATA_TRANSFER_H
