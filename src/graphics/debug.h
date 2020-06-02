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
extern dbg_tick_t tick_on;
extern bool dbg_window_visible;
void debug_handle_event(SDL_Event* event);
void actual_dbg_tick();
#define dbg_tick(tick_time) if (dbg_window_visible && tick_time == tick_on) { actual_dbg_tick(); }
void set_dbg_window_visibility(bool visible);
void dbg_line_drawn();
#endif //GBA_DEBUG_H
