#include <SDL.h>
#include "../arm7tdmi/arm7tdmi.h"
#include "ppu.h"
#include "../mem/gbabus.h"

#ifndef GBA_DEBUG_H
#define GBA_DEBUG_H
void debug_init(arm7tdmi_t* new_cpu, gba_ppu_t* new_ppu, gbabus_t* new_bus);
void debug_handle_event(SDL_Event* event);
void dbg_tick();
void set_dbg_window_visibility(bool visible);
#endif //GBA_DEBUG_H
