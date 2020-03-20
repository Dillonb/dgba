#ifndef GBA_PPU_H
#define GBA_PPU_H

#include <stdbool.h>
#include "common/util.h"

#define GBA_SCREEN_X 240
#define GBA_SCREEN_HBLANK 68
#define GBA_SCREEN_Y 160
#define GBA_SCREEN_VBLANK 68

typedef union DISPCNT {
    struct {
        bool obj_window_display:1;
        bool window1_display:1;
        bool window0_display:1;
        bool screen_display_bg3:1;
        bool screen_display_bg2:1;
        bool screen_display_bg1:1;
        bool screen_display_bg0:1;
        bool forced_blank:1;
        bool obj_character_vram_mapping:1;
        bool hblank_interval_free:1;
        bool display_frame_select:1;
        bool cgbmode:1;
        unsigned mode:2;
    };
    half raw;
} DISPCNT_t;

typedef union BG0CNT {
    half raw;
} BG0CNT_t;

typedef struct color {
    byte r;
    byte g;
    byte b;
} color_t;

typedef struct gba_ppu {
    // State
    int x;
    int y;
    color_t screen[GBA_SCREEN_X][GBA_SCREEN_Y];
    // Registers
    DISPCNT_t DISPCNT;
    BG0CNT_t BG0CNT;
} gba_ppu_t;

void write_dispcnt(gba_ppu_t* state, half value);
void write_bg0cnt(gba_ppu_t* state, half value);
gba_ppu_t* init_ppu();
void ppu_step(gba_ppu_t* ppu);

#endif //GBA_PPU_H
