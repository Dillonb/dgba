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

typedef union BGCNT {
    half raw;
} BGCNT_t;

typedef union BGOFS {
    struct {
        unsigned offset:9;
        unsigned:7;
    };
    half raw;
} BGOFS_t;

typedef union bg_rotation_scaling {
    struct {
        unsigned fractional:8;
        unsigned integer:7;
        bool sign:1;
    };
    half raw;
} bg_rotation_scaling_t;

typedef struct color {
    byte r;
    byte g;
    byte b;
} color_t;

typedef union DISPSTAT {
    struct {
        // Read only
        bool vblank:1;
        bool hblank:1;
        bool vcount:1;

        // R/W
        bool vblank_irq_enable:1;
        bool hblank_irq_enable:1;
        bool vcount_irq_enable:1;
        unsigned:1; // Unused: DSi LCD initialization ready (RO)
        unsigned:1; // Unused: NDS: MSB of v-vcount setting
        unsigned vcount_setting:8;
    };
    half raw;
} DISPSTAT_t;

typedef struct gba_ppu {
    // State
    int x;
    int y;
    color_t screen[GBA_SCREEN_X][GBA_SCREEN_Y];
    // Registers
    DISPCNT_t DISPCNT;

    BGCNT_t BG0CNT;
    BGCNT_t BG1CNT;
    BGCNT_t BG2CNT;
    BGCNT_t BG3CNT;

    BGOFS_t BG0HOFS;
    BGOFS_t BG0VOFS;
    BGOFS_t BG1HOFS;
    BGOFS_t BG1VOFS;
    BGOFS_t BG2HOFS;
    BGOFS_t BG2VOFS;
    BGOFS_t BG3HOFS;
    BGOFS_t BG3VOFS;

    bg_rotation_scaling_t BG2PA;
    bg_rotation_scaling_t BG2PB;
    bg_rotation_scaling_t BG2PC;
    bg_rotation_scaling_t BG2PD;

    DISPSTAT_t DISPSTAT;
} gba_ppu_t;

void write_dispcnt(gba_ppu_t* state, half value);
void write_bgcnt(gba_ppu_t* state, int num, half value);
gba_ppu_t* init_ppu();
void ppu_step(gba_ppu_t* ppu);

#endif //GBA_PPU_H
