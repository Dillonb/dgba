#ifndef GBA_RENDER_H
#define GBA_RENDER_H

#include <SDL.h>
#include "ppu.h"

void set_screen_scale(int scale);
void render_screen(color_t (*screen)[GBA_SCREEN_Y][GBA_SCREEN_X]);
void gba_handle_event(SDL_Event* event); // Only used so the debug window can send events back

#endif //GBA_RENDER_H
