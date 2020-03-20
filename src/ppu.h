#ifndef GBA_PPU_H
#define GBA_PPU_H

#include <stdbool.h>
#include "common/util.h"

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

typedef struct gba_ppu {
    // State
    int x;
    int y;
    // Registers
    DISPCNT_t DISPCNT;
} gba_ppu_t;

void write_dispcnt(gba_ppu_t* state, half value);
gba_ppu_t* init_ppu();

#endif //GBA_PPU_H
