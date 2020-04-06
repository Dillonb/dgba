#include <SDL.h>
#include "../arm7tdmi/arm7tdmi.h"
#include "ppu.h"
#include "../mem/gbabus.h"

#ifndef GBA_DEBUG_H
#define GBA_DEBUG_H
typedef enum dbg_tick {
    INSTRUCTION,
    SCANLINE,
    FRAME
} dbg_tick_t;
void debug_handle_event(SDL_Event* event);
void dbg_tick(dbg_tick_t tick_time);
void set_dbg_window_visibility(bool visible);
#endif //GBA_DEBUG_H
