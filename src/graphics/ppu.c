#include <assert.h>
#include "ppu.h"
#include "../common/log.h"
#include "../mem/gbabus.h"
#include "render.h"
#include "debug.h"
#include "../mem/dma.h"


typedef struct obj_affine {
    int16_t pa;
    int16_t pb;
    int16_t pc;
    int16_t pd;
} obj_affine_t;

gba_ppu_t* init_ppu() {
    assert(sizeof(float) == sizeof(word));

    gba_ppu_t* ppu = malloc(sizeof(gba_ppu_t));
    memset(ppu, 0, sizeof(gba_ppu_t));

    ppu->BG2PA.raw = 0x0100;
    ppu->BG2PB.raw = 0x0000;
    ppu->BG2PC.raw = 0x0000;
    ppu->BG2PA.raw = 0x0100;

    ppu->BG3PA.raw = 0x0100;
    ppu->BG3PB.raw = 0x0000;
    ppu->BG3PC.raw = 0x0000;
    ppu->BG3PA.raw = 0x0100;

    return ppu;
}

INLINE bool is_win(int x, int y, int x1, int x2, int y1, int y2) {
    return (x >= x1 && x <= x2) && (y >= y1 && y <= y2);
}

INLINE bool is_win0(gba_ppu_t* ppu, int x, int y) {
    return is_win(x, y, ppu->WIN0H.x1, ppu->WIN0H.x2, ppu->WIN0V.y1, ppu->WIN0V.y2);
}

INLINE bool is_win1(gba_ppu_t* ppu, int x, int y) {
    return is_win(x, y, ppu->WIN1H.x1, ppu->WIN1H.x2, ppu->WIN1V.y1, ppu->WIN1V.y2);
}

INLINE bool should_render_pixel_window(gba_ppu_t* ppu, int x, int y, bool win0in, bool win1in, bool winout, bool objin) {
    bool is_win0in = is_win0(ppu, x, y);
    bool is_win1in = is_win1(ppu, x, y);
    bool is_winobj = ppu->obj_window[x];
    bool is_winout = !(is_win0in || is_win1in);

    bool win0_display = ppu->DISPCNT.window0_display;
    bool win1_display = ppu->DISPCNT.window1_display;
    bool winobj_display = ppu->DISPCNT.obj_window_display;
    bool winout_display = win0_display || win1_display;

    if (win0_display && is_win0in) {
        return win0in;
    } else if (win1_display && is_win1in) {
        return win1in;
    } else if (winobj_display && is_winobj) {
        return objin;
    } else if (winout_display && is_winout) {
        return winout;
    }

    return true;
}

#define PALETTE_BANK_BACKGROUND 0

void render_line_mode3(gba_ppu_t* ppu) {
    if (ppu->DISPCNT.screen_display_bg2) {
        for (int x = 0; x < GBA_SCREEN_X; x++) {
            int offset = x + (ppu->y * GBA_SCREEN_X);
            offset *= 2;

            gba_color_t color;
            color.raw = half_from_byte_array(ppu->vram, offset) & 0x7FFF;

            ppu->screen[ppu->y][x].a = 0xFF;
            ppu->screen[ppu->y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(color.r);
            ppu->screen[ppu->y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(color.g);
            ppu->screen[ppu->y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(color.b);
        }
    } else {
        for (int x = 0; x < GBA_SCREEN_X; x++) {
            ppu->screen[ppu->y][x].a = 0;
            ppu->screen[ppu->y][x].r = 0;
            ppu->screen[ppu->y][x].g = 0;
            ppu->screen[ppu->y][x].b = 0;
        }
    }
}

// [shape][size]
int sprite_heights[3][4] = {
        {8,16,32,64},
        {8,8,16,32},
        {16,32,32,64}
};

// [shape][size]
int sprite_widths[3][4] = {
        {8,16,32,64},
        {16,32,32,64},
        {8,8,16,32}
};

#define OBJ_TILE_SIZE 0x20

#define OBJ_MODE_NORMAL  0b00
#define OBJ_MODE_ALPHA   0b01
#define OBJ_MODE_OBJWIN  0b10

#define OBJ_AFF_MODE_NORMAL 0b00
#define OBJ_AFF_MODE_AFFINE 0b01
#define OBJ_AFF_MODE_HIDE   0b10
#define OBJ_AFF_MODE_DOUBLE 0b11

void render_obj(gba_ppu_t* ppu) {
    obj_attr0_t attr0;
    obj_attr1_t attr1;
    obj_attr2_t attr2;
    for (int x = 0; x < GBA_SCREEN_X; x++) {
        ppu->obj_priorities[x] = 0;
        ppu->obj_window[x] = false;
        ppu->objbuf[x].raw = half_from_byte_array(ppu->pram, 0);
        ppu->objbuf[x].transparent = true;
    }

    for (int sprite = 0; sprite < 128; sprite++) {
        attr0.raw = half_from_byte_array(ppu->oam, (sprite * 8) + 0);
        attr1.raw = half_from_byte_array(ppu->oam, (sprite * 8) + 2);
        attr2.raw = half_from_byte_array(ppu->oam, (sprite * 8) + 4);

        int height = sprite_heights[attr0.shape][attr1.size];
        int width = sprite_widths[attr0.shape][attr1.size];
        int tiles_wide = width / 8;

        int hheight = height / 2;
        int hwidth = width / 2;

        bool is_double_affine = attr0.affine_object_mode == OBJ_AFF_MODE_DOUBLE;
        bool is_affine = attr0.affine_object_mode == OBJ_AFF_MODE_AFFINE || is_double_affine;

        int adjusted_x = attr1.x;
        int adjusted_y = attr0.y;

        if (is_double_affine) {
            adjusted_x += hwidth;
            adjusted_y += hheight;
        }

        if (adjusted_x >= 240) {
            adjusted_x -= 512;
        }
        if (adjusted_y >= 160) {
            adjusted_y -= 256;
        }

        int sprite_y = ppu->y - adjusted_y;
        if (!is_affine && attr1.vflip) {
            sprite_y = height - sprite_y - 1;
        }

        int screen_min_y = adjusted_y;
        int screen_max_y = adjusted_y + height;

        int screen_min_x = adjusted_x;
        int screen_max_x = adjusted_x + width;


        obj_affine_t affine;
        if (is_affine) {
            affine.pa = half_from_byte_array(ppu->oam, attr1.affine_index * 32 + 6);
            affine.pb = half_from_byte_array(ppu->oam, attr1.affine_index * 32 + 14);
            affine.pc = half_from_byte_array(ppu->oam, attr1.affine_index * 32 + 22);
            affine.pd = half_from_byte_array(ppu->oam, attr1.affine_index * 32 + 30);
            if (is_double_affine) { // double rendering area
                screen_min_y -= hheight;
                screen_max_y += hheight;

                screen_min_x -= hwidth;
                screen_max_x += hwidth;

                screen_min_x -= (width / 2);
                if (screen_min_x < 0) {
                    screen_min_x = 0;
                }
                screen_max_x += (width / 2);
                if (screen_max_x < GBA_SCREEN_X) {
                    screen_max_x = GBA_SCREEN_X;
                }
            }

        } else {
            // Set to identity matrix
            affine.pa = 0xFF;
            affine.pb = 0x00;
            affine.pc = 0x00;
            affine.pd = 0xFF;
        }

        if (ppu->y >= screen_min_y && ppu->y < screen_max_y) { // If the sprite is visible, we should draw it.
            if (attr0.affine_object_mode != OBJ_AFF_MODE_HIDE) { // Not disabled
                int sprite_x_start = is_double_affine ? -hwidth : 0;
                int sprite_x_end   = is_double_affine ? width + hwidth : width;
                for (int sprite_x = sprite_x_start; sprite_x < sprite_x_end; sprite_x++) {
                    int adjusted_sprite_x = sprite_x;
                    int adjusted_sprite_y = sprite_y;

                    if (is_affine) {
                        adjusted_sprite_x = affine.pa * (sprite_x - hwidth) + affine.pb * (sprite_y - hheight);
                        adjusted_sprite_x >>= 8;
                        adjusted_sprite_x += hwidth;

                        if (adjusted_sprite_x >= width || adjusted_sprite_x < 0) {
                            continue;
                        }

                        adjusted_sprite_y = affine.pc * (sprite_x - hwidth) + affine.pd * (sprite_y - hheight);
                        adjusted_sprite_y >>= 8;
                        adjusted_sprite_y += hheight;

                        if (adjusted_sprite_y >= height || adjusted_sprite_y < 0) {
                            continue;
                        }

                    } else if (attr1.hflip) {
                        adjusted_sprite_x = width - sprite_x - 1;
                    }

                    int y_tid_offset;
                    int sprite_tile_y = adjusted_sprite_y / 8;
                    if (ppu->DISPCNT.obj_character_vram_mapping) { // 1D
                        // Tiles are twice as wide in 256 color mode
                        y_tid_offset = tiles_wide * (sprite_tile_y << attr0.is_256color);
                    } else { // 2D
                        y_tid_offset = 32 * sprite_tile_y;
                    }
                    // After adding this offset, we won't need to worry about 1D vs 2D,
                    // because in either case they'll be right next to each other in memory.
                    int tid = attr2.tid + y_tid_offset;

                    // Don't use the adjusted X or Y here. There'd be no point in transforming the sprite, otherwise.
                    int screen_x = sprite_x + adjusted_x;
                    // Only draw if we've never drawn anything there before. Lower indices have higher priority
                    // and that's the order we're drawing them here.
                    if (screen_x < GBA_SCREEN_X && screen_x >= 0 && screen_x >= screen_min_x && screen_x < screen_max_x && (ppu->objbuf[screen_x].transparent || attr2.priority < ppu->obj_priorities[screen_x])) {
                        // Tiles are twice as wide in 256 color mode
                        int x_tid_offset = (adjusted_sprite_x / 8) << attr0.is_256color;
                        int tid_offset_by_x = tid + x_tid_offset;
                        word tile_address = 0x10000 + tid_offset_by_x * OBJ_TILE_SIZE;

                        int in_tile_x = adjusted_sprite_x % 8;
                        int in_tile_y = adjusted_sprite_y % 8;

                        int in_tile_offset = in_tile_x + in_tile_y * 8;
                        tile_address += in_tile_offset >> (!attr0.is_256color);

                        byte tile = ppu->vram[tile_address];
                        if (!attr0.is_256color) {
                            tile >>= (in_tile_offset % 2) * 4;
                            tile &= 0xF;
                        }

                        if (tile != 0) {

                            word palette_address = 0x200; // OBJ palette base
                            if (attr0.is_256color) {
                                palette_address += 2 * tile;
                            } else {
                                palette_address += (0x20 * attr2.pb + 2 * tile);
                            }
                            if (attr0.graphics_mode == OBJ_MODE_OBJWIN) {
                                ppu->obj_window[screen_x] = true;
                            } else {
                                if (should_render_pixel_window(ppu, screen_x, ppu->y, ppu->WININ.win0_obj_enable, ppu->WININ.win1_obj_enable, ppu->WINOUT.outside_obj_enable, ppu->WINOUT.obj_obj_enable)) {
                                    ppu->obj_priorities[screen_x] = attr2.priority;
                                    ppu->obj_alpha[screen_x] = attr0.graphics_mode == OBJ_MODE_ALPHA;
                                    ppu->objbuf[screen_x].raw = half_from_byte_array(ppu->pram, palette_address);
                                    ppu->objbuf[screen_x].transparent = false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

typedef union reg_se {
    half raw;
    struct {
        unsigned tid:10;
        bool hflip:1;
        bool vflip:1;
        unsigned pb:4;
    };
} reg_se_t;

INLINE void render_tile(gba_ppu_t* ppu, int tid, int pb, gba_color_t (*line)[GBA_SCREEN_X], int screen_x, bool is_256color, word character_base_addr, int tile_x, int tile_y) {
    int in_tile_offset_divisor = is_256color ? 1 : 2;
    int tile_size = is_256color ? 0x40 : 0x20;
    int in_tile_offset = tile_x + tile_y * 8;
    word tile_address = character_base_addr + tid * tile_size;
    tile_address += in_tile_offset / in_tile_offset_divisor;

    byte tile = ppu->vram[tile_address];

    if (!is_256color) {
        tile >>= (in_tile_offset % 2) * 4;
        tile &= 0xF;
    }

    word palette_address = is_256color ? 2 * tile : (0x20 * pb + 2 * tile);
    (*line)[screen_x].raw = half_from_byte_array(ppu->pram, palette_address);
    (*line)[screen_x].transparent = tile == 0; // This color should only be drawn if we need transparency
}

INLINE void render_screenentry(gba_ppu_t* ppu, gba_color_t (*line)[GBA_SCREEN_X], int screen_x, reg_se_t se, bool is_256color, word character_base_addr, int tilemap_x, int tilemap_y) {
    // Find the tile
    int tile_x = tilemap_x % 8;
    if (se.hflip) {
        tile_x = 7 - tile_x;
    }
    int tile_y = tilemap_y % 8;
    if (se.vflip) {
        tile_y = 7 - tile_y;
    }

    render_tile(ppu, se.tid, se.pb, line, screen_x, is_256color, character_base_addr, tile_x, tile_y);
}


#define SCREENBLOCK_SIZE 0x800
#define CHARBLOCK_SIZE  0x4000
INLINE void render_bg_regular(gba_ppu_t* ppu, gba_color_t (*line)[GBA_SCREEN_X], BGCNT_t* bgcnt, int hofs, int vofs, bool win0in, bool win1in, bool winout, bool objin) {
    // Tileset (like pattern tables in the NES)
    word character_base_addr = bgcnt->character_base_block * CHARBLOCK_SIZE;
    // Tile map (like nametables in the NES)
    word screen_base_addr = bgcnt->screen_base_block * SCREENBLOCK_SIZE;

    reg_se_t se;
    for (int x = 0; x < GBA_SCREEN_X; x++) {
        if (should_render_pixel_window(ppu, x, ppu->y, win0in, win1in, winout, objin)) {
            int screenblock_number;
            switch (bgcnt->screen_size) {
                case 0:
                    // 0
                    screenblock_number = 0;
                    break;
                case 1:
                    // 0 1
                    screenblock_number = ((x + hofs) % 512) > 255 ? 1 : 0;
                    break;
                case 2:
                    // 0
                    // 1
                    screenblock_number = ((ppu->y + vofs) % 512) > 255 ? 1 : 0;
                    break;
                case 3:
                    // 0 1
                    // 2 3
                    screenblock_number = ((x + hofs) % 512) > 255 ? 1 : 0;
                    screenblock_number += ((ppu->y + vofs) % 512) > 255 ? 2 : 0;
                    break;
                default:
                    logfatal("Unimplemented screen size: %d", bgcnt->screen_size);

            }
            int tilemap_x = (x + hofs) % 256;
            int tilemap_y = (ppu->y + vofs) % 256;

            int se_number = (tilemap_x / 8) + (tilemap_y / 8) * 32;
            se.raw = half_from_byte_array(ppu->vram,
                                          (screen_base_addr + screenblock_number * SCREENBLOCK_SIZE + se_number * 2));
            render_screenentry(ppu, line, x, se, bgcnt->is_256color, character_base_addr, tilemap_x, tilemap_y);
        } else {
            (*line)[x].raw = half_from_byte_array(ppu->pram, 0);
            (*line)[x].transparent = true;
        }
    }
}

void render_bg_affine(gba_ppu_t* ppu, gba_color_t (*line)[GBA_SCREEN_X], BGCNT_t* bgcnt,
                      bool win0in, bool win1in, bool winout, bool objin,
                      bg_referencepoint_container_t* x, bg_referencepoint_container_t* y,
                      bg_rotation_scaling_t* pa, bg_rotation_scaling_t* pb, bg_rotation_scaling_t* pc, bg_rotation_scaling_t* pd) {
    // Tileset (like pattern tables in the NES)
    word character_base_addr = bgcnt->character_base_block * CHARBLOCK_SIZE;
    // Tile map (like nametables in the NES)
    word screen_base_addr = bgcnt->screen_base_block * SCREENBLOCK_SIZE;

    int bg_width;
    int bg_height;
    switch (bgcnt->screen_size) {
        case 0:
            bg_width = 128;
            bg_height = 128;
            break;
        case 1:
            bg_width = 256;
            bg_height = 256;
            break;
        case 2:
            bg_width = 512;
            bg_height = 512;
            break;
        case 3:
            bg_width = 1024;
            bg_height = 1024;
            break;
        default:
            logfatal("Unimplemented screen size: %d", bgcnt->screen_size);
    }

    for (int screen_x = 0; screen_x < GBA_SCREEN_X; screen_x++) {
        word adjusted_x = (word)(x->current.sraw + pa->sraw * screen_x) >> 8;
        word adjusted_y = (word)(y->current.sraw + pc->sraw * screen_x) >> 8;

        if (bgcnt->wraparound) {
            adjusted_x %= bg_width;
            adjusted_y %= bg_height;

            if (adjusted_x < 0) {
                adjusted_x += bg_width;
            }
            if (adjusted_y < 0) {
                adjusted_y += bg_height;
            }
        }

        if (adjusted_y < bg_height && adjusted_x < bg_width && should_render_pixel_window(ppu, screen_x, ppu->y, win0in, win1in, winout, objin)) {
            int se_number = (adjusted_x / 8) + (adjusted_y / 8) * (bg_width / 8);
            byte tid = ppu->vram[screen_base_addr + se_number];
            render_tile(ppu, tid, 0, line, screen_x, true, character_base_addr, adjusted_x % 8, adjusted_y % 8);
        } else {
            (*line)[screen_x].raw = half_from_byte_array(ppu->pram, 0);
            (*line)[screen_x].transparent = true;
        }
    }
}

int background_priorities[4];

void refresh_background_priorities(gba_ppu_t* ppu) {
    int insert_index = 0;
    // Insert all backgrounds with a certain priority, counting up
    for (int priority = 0; priority < 4; priority++) {
        if (ppu->BG0CNT.priority == priority) {
            background_priorities[insert_index] = 0;
            insert_index++;
        }
        if (ppu->BG1CNT.priority == priority) {
            background_priorities[insert_index] = 1;
            insert_index++;
        }
        if (ppu->BG2CNT.priority == priority) {
            background_priorities[insert_index] = 2;
            insert_index++;
        }
        if (ppu->BG3CNT.priority == priority) {
            background_priorities[insert_index] = 3;
            insert_index++;
        }
    }
}

INLINE word word_min(word a, word b) {
    if (a < b) {
        return a;
    }
    return b;
}

INLINE gba_color_t blend(gba_color_t bottom, byte factor_bottom, gba_color_t top, byte factor_top) {
    gba_color_t blended;
    word new_r = bottom.r * factor_bottom + top.r * factor_top;
    word new_g = bottom.g * factor_bottom + top.g * factor_top;
    word new_b = bottom.b * factor_bottom + top.b * factor_top;

    blended.r = word_min(0x1F, new_r >> 4);
    blended.g = word_min(0x1F, new_g >> 4);
    blended.b = word_min(0x1F, new_b >> 4);

    return blended;
}

#define BG_OBJ 4
#define BG_BD  5

gba_color_t white = {{.r = 0x1F, .g = 0x1F, .b = 0x1F}};
gba_color_t black = {{.r = 0, .g = 0, .b = 0}};

INLINE void merge_bgs(gba_ppu_t* ppu) {
    byte eva = ppu->BLDALPHA.eva >= 0b10000 ? 0b10000 : ppu->BLDALPHA.eva;
    byte evb = ppu->BLDALPHA.evb >= 0b10000 ? 0b10000 : ppu->BLDALPHA.evb;
    byte ey  = ppu->BLDY.ey      >= 0b10000 ? 0b10000 : ppu->BLDY.ey;

    bool bg_enabled[] = {
            ppu->DISPCNT.screen_display_bg0,
            ppu->DISPCNT.screen_display_bg1,
            ppu->DISPCNT.screen_display_bg2,
            ppu->DISPCNT.screen_display_bg3};

    bool bg_top[] = {
            ppu->BLDCNT.aBG0,
            ppu->BLDCNT.aBG1,
            ppu->BLDCNT.aBG2,
            ppu->BLDCNT.aBG3,
            ppu->BLDCNT.aOBJ,
            ppu->BLDCNT.aBD
    };

    bool bg_bottom[] = {
            ppu->BLDCNT.bBG0,
            ppu->BLDCNT.bBG1,
            ppu->BLDCNT.bBG2,
            ppu->BLDCNT.bBG3,
            ppu->BLDCNT.bOBJ,
            ppu->BLDCNT.bBD
    };

    for (int x = 0; x < GBA_SCREEN_X; x++) {
        gba_color_t last;
        last.raw = half_from_byte_array(ppu->pram, 0);
        int last_layer_drawn = BG_BD;
        gba_color_t draw = last;

        bool should_blend_window = true;

        // TODO really should be cacheing all this window stuff and only doing it once per pixel/scanline
        bool win0 = is_win0(ppu, x, ppu->y);
        bool win1 = is_win1(ppu, x, ppu->y);
        bool winout = !(win0 || win1);

        bool win0_display = ppu->DISPCNT.window0_display;
        bool win1_display = ppu->DISPCNT.window1_display;
        bool winobj_display = ppu->DISPCNT.obj_window_display;
        bool winout_display = win0_display || win1_display;

        if (win0 && win0_display) {
            should_blend_window = ppu->WININ.win0_color_special_effect_enable;
        } else if (win1 && win1_display) {
            should_blend_window = ppu->WININ.win1_color_special_effect_enable;
        } else if (ppu->obj_window[x] && winobj_display) {
            should_blend_window = ppu->WINOUT.obj_color_special_effect_enable;
        } else if (winout && winout_display) {
            should_blend_window = ppu->WINOUT.outside_color_special_effect_enable;
        }

        for (int i = 3; i >= 0; i--) { // Draw them in reverse priority order, so the highest priority BG is drawn last.
            int bg = background_priorities[i];
            bool should_draw = bg_enabled[bg];

            bool should_blend_single = ppu->BLDCNT.blend_mode == BLD_BLACK || ppu->BLDCNT.blend_mode == BLD_WHITE;

            bool overlaps_target_pixel = bg_bottom[last_layer_drawn]; // last layer drawn is enabled for blending as a _bottom layer_

            bool should_blend_multiple = (ppu->BLDCNT.blend_mode == BLD_STD && overlaps_target_pixel);


            // current layer is enabled for drawing, blending as a _top layer_, and eligible to be blended given above conditions.
            bool should_blend = should_blend_window && (bg_top[bg] && (should_blend_multiple || should_blend_single));
            bool should_blend_obj = ppu->obj_alpha[x] && (overlaps_target_pixel || should_blend_single);
            bool force_obj_std_blend = overlaps_target_pixel;

            gba_color_t pixel = ppu->bgbuf[bg][x];
            if (pixel.transparent) {
                // If the pixel is transparent, don't draw it, since we already defaulted to the backdrop color.
                should_draw = false;
            }
            if (should_draw && should_blend && !pixel.transparent) {
                switch (ppu->BLDCNT.blend_mode) {
                    case BLD_OFF:
                        logfatal("Determined we should blend even though blending was off?")
                    case BLD_STD: {
                        draw = blend(last, evb, pixel, eva);
                        break;
                    }
                    case BLD_WHITE: {
                        draw = blend(white, ey, pixel, 16 - ey);
                        break;
                    }
                    case BLD_BLACK: {
                        draw = blend(black, ey, pixel, 16 - ey);
                        break;
                    }
                }
                last_layer_drawn = bg;
                last = pixel;
            } else if (should_draw) {
                last = pixel;
                draw = pixel;
                last_layer_drawn = bg;
            }
            // If the OBJ pixel here has the same priority as the BG, draw it instead.
            // "Sprites cover backgrounds of the same priority"
            if (ppu->obj_priorities[x] == i && !ppu->objbuf[x].transparent) {
                pixel = ppu->objbuf[x];
                if (should_blend_obj) {
                    byte obj_blend_mode = force_obj_std_blend ? BLD_STD : ppu->BLDCNT.blend_mode;
                    switch (obj_blend_mode) {
                        case BLD_OFF:
                            logfatal("Determined we should blend even though blending was off?")
                        case BLD_STD: {
                            draw = blend(last, evb, pixel, eva);
                            break;
                        }
                        case BLD_WHITE: {
                            draw = blend(white, ey, pixel, 16 - ey);
                            break;
                        }
                        case BLD_BLACK: {
                            draw = blend(black, ey, pixel, 16 - ey);
                            break;
                        }
                    }
                } else {
                    draw = pixel;
                }
                last = pixel;
                last_layer_drawn = BG_OBJ;
            }
        }
        ppu->screen[ppu->y][x].a = 0xFF;
        ppu->screen[ppu->y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(draw.r);
        ppu->screen[ppu->y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(draw.g);
        ppu->screen[ppu->y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(draw.b);
    }
}

INLINE void render_line_mode0(gba_ppu_t* ppu) {
    render_obj(ppu);
    if (ppu->DISPCNT.screen_display_bg0) {
        render_bg_regular(ppu, &ppu->bgbuf[0], &ppu->BG0CNT, ppu->BG0HOFS.offset, ppu->BG0VOFS.offset,
                          ppu->WININ.win0_bg0_enable, ppu->WININ.win1_bg0_enable, ppu->WINOUT.outside_bg0_enable, ppu->WINOUT.obj_bg0_enable);
    }

    if (ppu->DISPCNT.screen_display_bg1) {
        render_bg_regular(ppu, &ppu->bgbuf[1], &ppu->BG1CNT, ppu->BG1HOFS.offset, ppu->BG1VOFS.offset,
                          ppu->WININ.win0_bg1_enable, ppu->WININ.win1_bg1_enable, ppu->WINOUT.outside_bg1_enable, ppu->WINOUT.obj_bg1_enable);
    }

    if (ppu->DISPCNT.screen_display_bg2) {
        render_bg_regular(ppu, &ppu->bgbuf[2], &ppu->BG2CNT, ppu->BG2HOFS.offset, ppu->BG2VOFS.offset,
                          ppu->WININ.win0_bg2_enable, ppu->WININ.win1_bg2_enable, ppu->WINOUT.outside_bg2_enable, ppu->WINOUT.obj_bg2_enable);
    }

    if (ppu->DISPCNT.screen_display_bg3) {
        render_bg_regular(ppu, &ppu->bgbuf[3], &ppu->BG3CNT, ppu->BG3HOFS.offset, ppu->BG3VOFS.offset,
                          ppu->WININ.win0_bg3_enable, ppu->WININ.win1_bg3_enable, ppu->WINOUT.outside_bg3_enable, ppu->WINOUT.obj_bg3_enable);
    }

    refresh_background_priorities(ppu);

    merge_bgs(ppu);
}

INLINE void render_line_mode1(gba_ppu_t* ppu) {
    render_obj(ppu);

    if (ppu->DISPCNT.screen_display_bg0) {
        render_bg_regular(ppu, &ppu->bgbuf[0], &ppu->BG0CNT, ppu->BG0HOFS.offset, ppu->BG0VOFS.offset,
                          ppu->WININ.win0_bg0_enable, ppu->WININ.win1_bg0_enable, ppu->WINOUT.outside_bg0_enable, ppu->WINOUT.obj_bg0_enable);
    }

    if (ppu->DISPCNT.screen_display_bg1) {
        render_bg_regular(ppu, &ppu->bgbuf[1], &ppu->BG1CNT, ppu->BG1HOFS.offset, ppu->BG1VOFS.offset,
                          ppu->WININ.win0_bg1_enable, ppu->WININ.win1_bg1_enable, ppu->WINOUT.outside_bg1_enable, ppu->WINOUT.obj_bg1_enable);
    }

    if (ppu->DISPCNT.screen_display_bg2) {
        render_bg_affine(ppu, &ppu->bgbuf[2], &ppu->BG2CNT,
                         ppu->WININ.win0_bg2_enable, ppu->WININ.win1_bg2_enable, ppu->WINOUT.outside_bg2_enable, ppu->WINOUT.obj_bg2_enable,
                         &ppu->BG2X, &ppu->BG2Y, &ppu->BG2PA, &ppu->BG2PB, &ppu->BG2PC, &ppu->BG2PD);
    }

    refresh_background_priorities(ppu);

    merge_bgs(ppu);
}

INLINE void render_line_mode2(gba_ppu_t* ppu) {
    render_obj(ppu);

    if (ppu->DISPCNT.screen_display_bg2) {
        render_bg_affine(ppu, &ppu->bgbuf[2], &ppu->BG2CNT,
                         ppu->WININ.win0_bg2_enable, ppu->WININ.win1_bg2_enable, ppu->WINOUT.outside_bg2_enable, ppu->WINOUT.obj_bg2_enable,
                         &ppu->BG2X, &ppu->BG2Y, &ppu->BG2PA, &ppu->BG2PB, &ppu->BG2PC, &ppu->BG2PD);
    }

    if (ppu->DISPCNT.screen_display_bg3) {
        render_bg_affine(ppu, &ppu->bgbuf[3], &ppu->BG3CNT,
                         ppu->WININ.win0_bg3_enable, ppu->WININ.win1_bg3_enable, ppu->WINOUT.outside_bg3_enable, ppu->WINOUT.obj_bg3_enable,
                         &ppu->BG3X, &ppu->BG3Y, &ppu->BG3PA, &ppu->BG3PB, &ppu->BG3PC, &ppu->BG3PD);
    }

    refresh_background_priorities(ppu);

    merge_bgs(ppu);
}

void render_line_mode4(gba_ppu_t* ppu) {
    render_obj(ppu);
    if (ppu->DISPCNT.screen_display_bg2) {
        for (int x = 0; x < GBA_SCREEN_X; x++) {
            int offset = x + (ppu->y * GBA_SCREEN_X);
            int index = ppu->DISPCNT.display_frame_select * 0xA000 + offset;
            int tile = ppu->vram[index];
            if (tile == 0) {
                ppu->bgbuf[2][x].raw = half_from_byte_array(ppu->pram, 0);
                ppu->bgbuf[2][x].transparent = true;
            } else {
                ppu->bgbuf[2][x].raw = half_from_byte_array(ppu->pram, (0x20 * PALETTE_BANK_BACKGROUND + 2 * tile)) & 0x7FFF;
                ppu->bgbuf[2][x].transparent = false;
            }
        }
    } else {
        for (int x = 0; x < GBA_SCREEN_X; x++) {
            ppu->bgbuf[2][x].raw = half_from_byte_array(ppu->pram, 0);
            ppu->bgbuf[2][x].transparent = true;
        }
    }
    refresh_background_priorities(ppu);
    merge_bgs(ppu);
}


INLINE void render_line(gba_ppu_t* ppu) {
    // Draw a pixel
    switch (ppu->DISPCNT.mode) {
        case 0:
            render_line_mode0(ppu);
            break;
        case 1:
            render_line_mode1(ppu);
            break;
        case 2:
            render_line_mode2(ppu);
            break;
        case 3:
            render_line_mode3(ppu);
            break;
        case 4:
            render_line_mode4(ppu);
            break;
        default:
            logfatal("Unknown graphics mode: %d", ppu->DISPCNT.mode)
    }
}

void ppu_hblank(gba_ppu_t* ppu) {
    if (!is_vblank(ppu)) {
        dma_start_trigger(HBlank);
    }
    if (ppu->DISPSTAT.hblank_irq_enable) {
        request_interrupt(IRQ_HBLANK);
    }
    ppu->DISPSTAT.hblank = true;
    if (ppu->y < GBA_SCREEN_Y && !ppu->DISPCNT.forced_blank) { // i.e. not VBlank
        render_line(ppu);
    }
}

void ppu_vblank(gba_ppu_t* ppu) {
    dma_start_trigger(VBlank);
    if (ppu->DISPSTAT.vblank_irq_enable) {
        request_interrupt(IRQ_VBLANK);
    }
    ppu->DISPSTAT.vblank = true;
    render_screen(&ppu->screen);
}

void ppu_end_hblank(gba_ppu_t* ppu) {
    dbg_tick(SCANLINE);

    ppu->BG2X.current.sraw += ppu->BG2PB.sraw;
    ppu->BG3X.current.sraw += ppu->BG3PB.sraw;
    ppu->BG2Y.current.sraw += ppu->BG2PD.sraw;
    ppu->BG3Y.current.sraw += ppu->BG3PD.sraw;

    ppu->DISPSTAT.hblank = false;
    ppu->y++;

    if (ppu->y == ppu->DISPSTAT.vcount_setting) {
        ppu->DISPSTAT.vcount = true;
        if (ppu->DISPSTAT.vcount_irq_enable) {
            request_interrupt(IRQ_VCOUNT);
        }
    } else {
        ppu->DISPSTAT.vcount = false;
    }
}

void ppu_end_vblank(gba_ppu_t* ppu) {
    ppu->BG2X.current.raw = ppu->BG2X.initial.raw;
    ppu->BG3X.current.raw = ppu->BG3X.initial.raw;
    ppu->BG2Y.current.raw = ppu->BG2Y.initial.raw;
    ppu->BG3Y.current.raw = ppu->BG3Y.initial.raw;

    ppu->y = 0;
    dbg_tick(FRAME);
    ppu->DISPSTAT.vblank = false;
}
