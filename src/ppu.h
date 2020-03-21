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

typedef union bg_referencepoint {
    struct {
        unsigned fractional:8;
        unsigned integer:19;
        bool sign:1;
        unsigned:4;
    };
    word raw;
} bg_referencepoint_t;

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

typedef union WINH {
    struct {
        unsigned x2:8;
        unsigned x1:8;
    };
    half raw;
} WINH_t;

typedef union WINV {
    struct {
        unsigned y2:8;
        unsigned y1:8;
    };
    half raw;
} WINV_t;

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
    bg_rotation_scaling_t BG3PA;
    bg_rotation_scaling_t BG3PB;
    bg_rotation_scaling_t BG3PC;
    bg_rotation_scaling_t BG3PD;

    WINH_t WIN0H;
    WINH_t WIN1H;
    WINV_t WIN0V;
    WINV_t WIN1V;

    bg_referencepoint_t BG2X;
    bg_referencepoint_t BG2Y;
    bg_referencepoint_t BG3X;
    bg_referencepoint_t BG3Y;

    DISPSTAT_t DISPSTAT;
} gba_ppu_t;

gba_ppu_t* init_ppu();
void ppu_step(gba_ppu_t* ppu);

#endif //GBA_PPU_H
