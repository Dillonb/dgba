#ifndef GBA_RENDER_H
#define GBA_RENDER_H

#include "ppu.h"

void render_screen(color_t (*screen)[GBA_SCREEN_Y][GBA_SCREEN_X]);

#endif //GBA_RENDER_H
