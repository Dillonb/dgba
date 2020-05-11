#ifndef GBA_PPU_H
#define GBA_PPU_H

#include <stdbool.h>
#include "../common/util.h"

#define GBA_SCREEN_X 240
#define GBA_SCREEN_HBLANK 68
#define GBA_SCREEN_Y 160
#define GBA_SCREEN_VBLANK 68

#define PRAM_SIZE  0x400
#define VRAM_SIZE  0x18000
#define OAM_SIZE   0x400

#define FIVEBIT_TO_EIGHTBIT_COLOR(c) (c<<3)|(c&7)

typedef union DISPCNT {
    struct {
        unsigned mode:3;
        bool cgbmode:1;
        bool display_frame_select:1;
        bool hblank_interval_free:1;
        bool obj_character_vram_mapping:1;
        bool forced_blank:1;
        bool screen_display_bg0:1;
        bool screen_display_bg1:1;
        bool screen_display_bg2:1;
        bool screen_display_bg3:1;
        bool screen_display_obj:1;
        bool window0_display:1;
        bool window1_display:1;
        bool obj_window_display:1;
    };
    half raw;
} DISPCNT_t;

typedef union BGCNT {
    struct {
        unsigned priority:2;
        unsigned character_base_block:2;
        unsigned:2;
        bool mosaic:1;
        unsigned is_256color:1;
        unsigned screen_base_block:5;
        bool wraparound:1;
        unsigned screen_size:2;
    };
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
    int16_t sraw;
} bg_rotation_scaling_t;

typedef union bg_referencepoint {
    struct {
        unsigned fractional:8;
        unsigned integer:19;
        bool sign:1;
        unsigned:4;
    };
    word raw;
    int32_t sraw;
} bg_referencepoint_t;

typedef struct bg_referencepoint_container {
    bg_referencepoint_t initial;
    bg_referencepoint_t current;
} bg_referencepoint_container_t;

INLINE double REF_TO_DOUBLE(bg_referencepoint_t ref) {
    return ((ref.sign ? -1 : 1) * ref.integer);
}

INLINE double ROTSCALE_TO_DOUBLE(bg_rotation_scaling_t* rs) {
    return ((rs->sign ? -1 : 1) * rs->integer);
}

typedef struct color {
    byte a;
    byte r;
    byte g;
    byte b;
} color_t;

typedef union gba_color {
    struct {
        unsigned r:5;
        unsigned g:5;
        unsigned b:5;
        bool transparent:1;
    } __attribute__ ((__packed__));
    half raw;
} gba_color_t;

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

typedef union WININ {
    struct {
        bool win0_bg0_enable:1;
        bool win0_bg1_enable:1;
        bool win0_bg2_enable:1;
        bool win0_bg3_enable:1;
        bool win0_obj_enable:1;
        bool win0_color_special_effect_enable:1;
        unsigned:2;
        bool win1_bg0_enable:1;
        bool win1_bg1_enable:1;
        bool win1_bg2_enable:1;
        bool win1_bg3_enable:1;
        bool win1_obj_enable:1;
        bool win1_color_special_effect_enable:1;
        unsigned:2;
    };
    half raw;
} WININ_t;

typedef union WINOUT {
    struct {
        bool outside_bg0_enable:1;
        bool outside_bg1_enable:1;
        bool outside_bg2_enable:1;
        bool outside_bg3_enable:1;
        bool outside_obj_enable:1;
        bool outside_color_special_effect_enable:1;
        unsigned:2;
        bool obj_bg0_enable:1;
        bool obj_bg1_enable:1;
        bool obj_bg2_enable:1;
        bool obj_bg3_enable:1;
        bool obj_obj_enable:1;
        bool obj_color_special_effect_enable:1;
        unsigned:2;
    };
    half raw;
} WINOUT_t;

typedef union MOSAIC {
    struct {
        unsigned bg_hsize:4;
        unsigned bg_vsize:4;
        unsigned obj_hsize:4;
        unsigned obj_vsize:4;
    };
    half raw;
} MOSAIC_t;

typedef union BLDCNT {
    struct {
        bool bg0_first_target_pixel:1;
        bool bg1_first_target_pixel:1;
        bool bg2_first_target_pixel:1;
        bool bg3_first_target_pixel:1;
        bool obj_first_target_pixel:1;
        bool bd_first_target_pixel:1;
        unsigned color_special_effect:2;
        bool bg0_second_target_pixel:1;
        bool bg1_second_target_pixel:1;
        bool bg2_second_target_pixel:1;
        bool bg3_second_target_pixel:1;
        bool obj_second_target_pixel:1;
        bool bd_second_target_pixel:1;
        unsigned:2;
    };
    half raw;
} BLDCNT_t;

typedef union BLDALPHA {
    half raw;
} BLDALPHA_t;

typedef union BLDY {
    half raw;
} BLDY_t;

typedef union addr_27b {
    struct {
        unsigned addr:27;
        unsigned:5;
    };
    word raw;
} addr_27b_t;

typedef union addr_28b {
    struct {
        unsigned addr:28;
        unsigned:4;
    };
    word raw;
} addr_28b_t;

typedef struct gba_ppu {
    // State
    half y;
    color_t screen[GBA_SCREEN_Y][GBA_SCREEN_X];
    gba_color_t bgbuf[4][GBA_SCREEN_X];
    gba_color_t objbuf[GBA_SCREEN_X];
    byte obj_priorities[GBA_SCREEN_X];

    // Memory
    byte pram[PRAM_SIZE];
    byte vram[VRAM_SIZE];
    byte oam[OAM_SIZE];


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
    WININ_t WININ;
    WINOUT_t WINOUT;
    MOSAIC_t MOSAIC;

    BLDCNT_t BLDCNT;
    BLDALPHA_t BLDALPHA;
    BLDY_t BLDY;

    bg_referencepoint_container_t BG2X;
    bg_referencepoint_container_t BG2Y;
    bg_referencepoint_container_t BG3X;
    bg_referencepoint_container_t BG3Y;

    DISPSTAT_t DISPSTAT;
} gba_ppu_t;

typedef union obj_attr0 {
    struct {
        unsigned y:8;
        unsigned affine_object_mode:2;
        unsigned graphics_mode:2;
        bool mosaic:1;
        bool is_256color:1;
        unsigned shape:2;
    };
    half raw;
} obj_attr0_t;

typedef union obj_attr1 {
    struct {
        unsigned x:9;
        unsigned affine_index:5;
        unsigned size:2;
    };
    struct {
        unsigned:12;
        bool hflip:1;
        bool vflip:1;
        unsigned:2;
    };
    half raw;
} obj_attr1_t;

typedef union obj_attr2 {
    struct {
        unsigned tid:10;
        unsigned priority:2;
        unsigned pb:4;
    };
    half raw;
} obj_attr2_t;

extern int sprite_heights[3][4];
extern int sprite_widths[3][4];


gba_ppu_t* init_ppu();
void ppu_hblank(gba_ppu_t* ppu);
void ppu_vblank(gba_ppu_t* ppu);
void ppu_end_hblank(gba_ppu_t* ppu);
void ppu_end_vblank(gba_ppu_t* ppu);

INLINE bool is_vblank(gba_ppu_t* ppu) {
    return ppu->y > GBA_SCREEN_Y && ppu->y != 227;
}

#endif //GBA_PPU_H
