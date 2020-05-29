#include <stdbool.h>

#include "../common/util.h"

#ifndef GBA_MGBA_DEBUG_H
#define GBA_MGBA_DEBUG_H
#ifdef ENABLE_MGBA_DEBUG

void mgba_debug_write_half(word address, half value);
half mgba_debug_read_half(word address);
bool is_mgba_debug_enabled();
void mgba_debug_write_byte(word address, byte value);

#endif //ENABLE_MGBA_DEBUG
#endif //GBA_MGBA_DEBUG_H
