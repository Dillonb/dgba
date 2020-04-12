#include "ppu.h"
#include "../common/log.h"
#include "../mem/gbabus.h"
#include "render.h"
#include "debug.h"
#include "../mem/dma.h"

gba_color_t bgbuf[4][GBA_SCREEN_X];
gba_color_t objbuf[GBA_SCREEN_X];
byte obj_priorities[GBA_SCREEN_X];

typedef struct obj_affine {
    int16_t pa;
    int16_t pb;
    int16_t pc;
    int16_t pd;
} obj_affine_t;

gba_ppu_t* init_ppu() {
    gba_ppu_t* ppu = malloc(sizeof(gba_ppu_t));

    ppu->DISPCNT.raw = 0;
    ppu->DISPSTAT.raw = 0;

    ppu->BG0CNT.raw = 0;
    ppu->BG1CNT.raw = 0;
    ppu->BG2CNT.raw = 0;
    ppu->BG3CNT.raw = 0;

    ppu->BG0HOFS.raw = 0;
    ppu->BG0VOFS.raw = 0;
    ppu->BG1HOFS.raw = 0;
    ppu->BG1VOFS.raw = 0;
    ppu->BG2HOFS.raw = 0;
    ppu->BG2VOFS.raw = 0;
    ppu->BG3HOFS.raw = 0;
    ppu->BG3VOFS.raw = 0;

    ppu->x = 0;
    ppu->y = 0;

    for (int i = 0; i < VRAM_SIZE; i++) {
        ppu->vram[i] = 0;
    }

    for (int i = 0; i < PRAM_SIZE; i++) {
        ppu->pram[i] = 0;
    }

    for (int i = 0; i < OAM_SIZE; i++) {
        ppu->oam[i] = 0;
    }

    return ppu;
}

INLINE bool is_hblank(gba_ppu_t* ppu) {
    return ppu->x > GBA_SCREEN_X;
}

INLINE bool is_vblank(gba_ppu_t* ppu) {
    return ppu->y > GBA_SCREEN_Y && ppu->y != 227;
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

#define PALETTE_BANK_BACKGROUND 0

void render_line_mode3(gba_ppu_t* ppu) {
    if (ppu->DISPCNT.screen_display_bg2) {
        for (int x = 0; x < GBA_SCREEN_X; x++) {
            int offset = x + (ppu->y * GBA_SCREEN_X); // Calculate this based on BG2X/Y/VOFS/HOFS/etc
            offset *= 2;

            gba_color_t color;
            color.raw = gba_read_half(0x06000000 + offset) & 0x7FFF;

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

void render_line_mode4(gba_ppu_t* ppu) {
    if (ppu->DISPCNT.screen_display_bg2) {
        for (int x = 0; x < GBA_SCREEN_X; x++) {
            int offset = x + (ppu->y * GBA_SCREEN_X); // Calculate this based on BG2X/Y/VOFS/HOFS/etc
            int index = ppu->DISPCNT.display_frame_select * 0xA000 + offset;
            int tile = ppu->vram[index];
            if (tile == 0) {
                ppu->screen[ppu->y][x].a = 0;
                ppu->screen[ppu->y][x].r = 0;
                ppu->screen[ppu->y][x].g = 0;
                ppu->screen[ppu->y][x].b = 0;
            } else {
                gba_color_t color;
                color.raw = gba_read_half(0x05000000 | (0x20 * PALETTE_BANK_BACKGROUND + 2 * tile)) & 0x7FFF;

                ppu->screen[ppu->y][x].a = 0xFF;
                ppu->screen[ppu->y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(color.r);
                ppu->screen[ppu->y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(color.g);
                ppu->screen[ppu->y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(color.b);
            }
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
void render_obj(gba_ppu_t* ppu) {
    obj_attr0_t attr0;
    obj_attr1_t attr1;
    obj_attr2_t attr2;
    for (int x = 0; x < GBA_SCREEN_X; x++) {
        obj_priorities[x] = 0;
        objbuf[x].transparent = true;
        objbuf[x].r = 0;
        objbuf[x].g = 0;
        objbuf[x].b = 0;
    }

    for (int sprite = 0; sprite < 128; sprite++) {
        attr0.raw = gba_read_half(0x07000000 + (sprite * 8) + 0);
        attr1.raw = gba_read_half(0x07000000 + (sprite * 8) + 2);
        attr2.raw = gba_read_half(0x07000000 + (sprite * 8) + 4);

        int height = sprite_heights[attr0.shape][attr1.size];
        int width = sprite_widths[attr0.shape][attr1.size];
        int tiles_wide = width / 8;

        int hheight = height / 2;
        int hwidth = width / 2;

        bool is_double_affine = attr0.affine_object_mode == 0b11;
        bool is_affine = attr0.affine_object_mode == 0b01 || is_double_affine;

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

        int in_tile_offset_divisor = attr0.is_256color ? 1 : 2;

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
            affine.pa = gba_read_half(0x07000000 + attr1.affine_index * 32 + 6);
            affine.pb = gba_read_half(0x07000000 + attr1.affine_index * 32 + 14);
            affine.pc = gba_read_half(0x07000000 + attr1.affine_index * 32 + 22);
            affine.pd = gba_read_half(0x07000000 + attr1.affine_index * 32 + 30);
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
            if (attr0.affine_object_mode != 0b10) { // Not disabled
                unimplemented(attr0.is_256color, "256 color sprite")
                int sprite_x_start = is_double_affine ? -hwidth : 0;
                int sprite_x_end   = is_double_affine ? width + hwidth : width;
                for (int sprite_x = sprite_x_start; sprite_x < sprite_x_end; sprite_x++) {
                    int adjusted_sprite_x = sprite_x;
                    int adjusted_sprite_y = sprite_y;

                    if (is_affine) {
                        adjusted_sprite_x = affine.pa * (sprite_x - hwidth) + affine.pb * (sprite_y - hheight);
                        adjusted_sprite_x >>= 8;
                        adjusted_sprite_x += hwidth;

                        if (adjusted_sprite_x > width || adjusted_sprite_x < 0) {
                            continue;
                        }

                        adjusted_sprite_y = affine.pc * (sprite_x - hwidth) + affine.pd * (sprite_y - hheight);
                        adjusted_sprite_y >>= 8;
                        adjusted_sprite_y += hheight;

                        if (adjusted_sprite_y > height || adjusted_sprite_y < 0) {
                            continue;
                        }

                    } else if (attr1.hflip) {
                        adjusted_sprite_x = width - sprite_x - 1;
                    }

                    int y_tid_offset;
                    int sprite_tile_y = adjusted_sprite_y / 8;
                    if (ppu->DISPCNT.obj_character_vram_mapping) { // 1D
                        y_tid_offset = tiles_wide * sprite_tile_y;
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
                    if (screen_x >= screen_min_x && screen_x < screen_max_x  && (objbuf[screen_x].transparent || attr2.priority < obj_priorities[screen_x])) {
                        int x_tid_offset = adjusted_sprite_x / 8;
                        int tid_offset_by_x = tid + x_tid_offset;
                        word tile_address = 0x06010000 + tid_offset_by_x * OBJ_TILE_SIZE;

                        int in_tile_x = adjusted_sprite_x % 8;
                        int in_tile_y = adjusted_sprite_y % 8;

                        int in_tile_offset = in_tile_x + in_tile_y * 8;
                        tile_address += in_tile_offset / in_tile_offset_divisor;

                        byte tile = gba_read_byte(tile_address);
                        if (!attr0.is_256color) {
                            tile >>= (in_tile_offset % 2) * 4;
                            tile &= 0xF;
                        }

                        if (tile != 0) {

                            word palette_address = 0x05000200; // OBJ palette base
                            if (attr0.is_256color) {
                                palette_address += 2 * tile;
                            } else {
                                palette_address += (0x20 * attr2.pb + 2 * tile);
                            }
                            obj_priorities[screen_x] = attr2.priority;
                            objbuf[screen_x].raw = gba_read_half(palette_address);
                            objbuf[screen_x].transparent = false;
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

INLINE void render_tile(int tid, int pb, gba_color_t (*line)[GBA_SCREEN_X], int screen_x, bool is_256color, word character_base_addr, int tile_x, int tile_y) {
    int in_tile_offset_divisor = is_256color ? 1 : 2;
    int tile_size = is_256color ? 0x40 : 0x20;
    int in_tile_offset = tile_x + tile_y * 8;
    word tile_address = character_base_addr + tid * tile_size;
    tile_address += in_tile_offset / in_tile_offset_divisor;

    byte tile = gba_read_byte(tile_address);

    if (!is_256color) {
        tile >>= (in_tile_offset % 2) * 4;
        tile &= 0xF;
    }

    word palette_address = 0x05000000;
    if (is_256color) {
        palette_address += 2 * tile;
    } else {
        palette_address += (0x20 * pb + 2 * tile);
    }
    (*line)[screen_x].raw = gba_read_half(palette_address);
    (*line)[screen_x].transparent = tile == 0; // This color should only be drawn if we need transparency
}

INLINE void render_screenentry(gba_color_t (*line)[GBA_SCREEN_X], int screen_x, reg_se_t se, bool is_256color, word character_base_addr, int tilemap_x, int tilemap_y) {
    // Find the tile
    int tile_x = tilemap_x % 8;
    if (se.hflip) {
        tile_x = 7 - tile_x;
    }
    int tile_y = tilemap_y % 8;
    if (se.vflip) {
        tile_y = 7 - tile_y;
    }

    render_tile(se.tid, se.pb, line, screen_x, is_256color, character_base_addr, tile_x, tile_y);
}

INLINE bool should_render_bg_pixel(gba_ppu_t* ppu, int x, int y, bool win0in, bool win1in, bool winout, bool objout) {
    bool is_win0in = is_win0(ppu, x, y);
    bool is_win1in = is_win1(ppu, x, y);
    bool is_winout = !(is_win0in || is_win1in);

    bool win0_enable = ppu->DISPCNT.window0_display;
    bool win1_enable = ppu->DISPCNT.window1_display;
    bool winout_enable = win0_enable || win1_enable;

    if (win0_enable && is_win0in && !win0in) {
        return false;
    }

    if (win1_enable && is_win1in && !win1in) {
        return false;
    }

    if (winout_enable && is_winout && !winout) {
        return false;
    }

    return true;
}

#define SCREENBLOCK_SIZE 0x800
#define CHARBLOCK_SIZE  0x4000
INLINE void render_bg_regular(gba_ppu_t* ppu, gba_color_t (*line)[GBA_SCREEN_X], BGCNT_t* bgcnt, int hofs, int vofs, bool win0in, bool win1in, bool winout, bool objout) {
    // Tileset (like pattern tables in the NES)
    word character_base_addr = 0x06000000 + bgcnt->character_base_block * CHARBLOCK_SIZE;
    // Tile map (like nametables in the NES)
    word screen_base_addr = 0x06000000 + bgcnt->screen_base_block * SCREENBLOCK_SIZE;


    reg_se_t se;
    for (int x = 0; x < GBA_SCREEN_X; x++) {
        if (should_render_bg_pixel(ppu, x, ppu->y, win0in, win1in, winout, objout)) {
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
            se.raw = gba_read_half(screen_base_addr + screenblock_number * SCREENBLOCK_SIZE + se_number * 2);
            render_screenentry(line, x, se, bgcnt->is_256color, character_base_addr, tilemap_x, tilemap_y);
        } else {
            (*line)[x].r = 0;
            (*line)[x].g = 0;
            (*line)[x].b = 0;
            (*line)[x].transparent = true;
        }
    }
}

#define REF_TO_DOUBLE(x) ((x->sign ? -1 : 1) * x->integer)
#define ROTSCALE_TO_DOUBLE(x) ((x->sign ? -1 : 1) * x->integer)

void render_bg_affine(gba_ppu_t* ppu, gba_color_t (*line)[GBA_SCREEN_X], BGCNT_t* bgcnt,
                      bool win0in, bool win1in, bool winout, bool objout,
                      bg_referencepoint_t* x, bg_referencepoint_t* y,
                      bg_rotation_scaling_t* pa, bg_rotation_scaling_t* pb, bg_rotation_scaling_t* pc, bg_rotation_scaling_t* pd) {
    // Tileset (like pattern tables in the NES)
    word character_base_addr = 0x06000000 + bgcnt->character_base_block * CHARBLOCK_SIZE;
    // Tile map (like nametables in the NES)
    word screen_base_addr = 0x06000000 + bgcnt->screen_base_block * SCREENBLOCK_SIZE;

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
        double adjusted_y;
        double adjusted_x;

        double ref_x = REF_TO_DOUBLE(x);
        double ref_y = REF_TO_DOUBLE(y);

        double d_pa = ROTSCALE_TO_DOUBLE(pa);
        double d_pb = ROTSCALE_TO_DOUBLE(pb);
        double d_pc = ROTSCALE_TO_DOUBLE(pc);
        double d_pd = ROTSCALE_TO_DOUBLE(pd);

        adjusted_x = (screen_x - ref_x) * d_pa + (ppu->y - ref_y) * d_pb;
        adjusted_x += ref_x;
        adjusted_y = (screen_x - ref_x) * d_pc + (ppu->y - ref_y) * d_pd;
        adjusted_y += ref_y;

        int render_x = (int)adjusted_x;
        int render_y = (int)adjusted_y;

        if (render_y < bg_height && render_x < bg_width && should_render_bg_pixel(ppu, screen_x, ppu->y, win0in, win1in, winout, objout)) {
            int se_number = (render_x / 8) + (render_y / 8) * (bg_width / 8);
            byte tid = gba_read_byte(screen_base_addr + se_number);
            render_tile(tid, 0, line, screen_x, true, character_base_addr, render_x % 8, render_y % 8);
        } else {
            (*line)[screen_x].r = 0;
            (*line)[screen_x].g = 0;
            (*line)[screen_x].b = 0;
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

INLINE void merge_bgs(gba_ppu_t* ppu) {
    bool bg_enabled[] = {
            ppu->DISPCNT.screen_display_bg0,
            ppu->DISPCNT.screen_display_bg1,
            ppu->DISPCNT.screen_display_bg2,
            ppu->DISPCNT.screen_display_bg3};

    for (int x = 0; x < GBA_SCREEN_X; x++) {
        bool non_transparent_drawn = false;
        for (int i = 3; i >= 0; i--) { // Draw them in reverse priority order, so the highest priority BG is drawn last.
            // If the OBJ pixel here has the same priority as the BG, draw it instead.
            // "Sprites cover backgrounds of the same priority"
            if (obj_priorities[x] == i && !objbuf[x].transparent) {
                ppu->screen[ppu->y][x].a = 0xFF;
                ppu->screen[ppu->y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(objbuf[x].r);
                ppu->screen[ppu->y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(objbuf[x].g);
                ppu->screen[ppu->y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(objbuf[x].b);
                non_transparent_drawn = true;
            } else {
                int bg = background_priorities[i];
                gba_color_t pixel = bgbuf[bg][x];
                bool should_draw = bg_enabled[bg];
                if (pixel.transparent) {
                    // If the pixel is transparent, only draw it if we haven't drawn a non-transparent
                    should_draw &= !non_transparent_drawn;
                }

                if (should_draw) {
                    ppu->screen[ppu->y][x].a = 0xFF;
                    ppu->screen[ppu->y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(pixel.r);
                    ppu->screen[ppu->y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(pixel.g);
                    ppu->screen[ppu->y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(pixel.b);

                    if (!pixel.transparent) {
                        non_transparent_drawn = true;
                    }
                }
            }
        }
    }
}

INLINE void render_line_mode0(gba_ppu_t* ppu) {
    render_obj(ppu);
    if (ppu->DISPCNT.screen_display_bg0) {
        render_bg_regular(ppu, &bgbuf[0], &ppu->BG0CNT, ppu->BG0HOFS.offset, ppu->BG0VOFS.offset,
                          ppu->WININ.win0_bg0_enable, ppu->WININ.win1_bg0_enable, ppu->WINOUT.outside_bg0_enable, ppu->WINOUT.obj_bg0_enable);
    }

    if (ppu->DISPCNT.screen_display_bg1) {
        render_bg_regular(ppu, &bgbuf[1], &ppu->BG1CNT, ppu->BG1HOFS.offset, ppu->BG1VOFS.offset,
                          ppu->WININ.win0_bg1_enable, ppu->WININ.win1_bg1_enable, ppu->WINOUT.outside_bg1_enable, ppu->WINOUT.obj_bg1_enable);
    }

    if (ppu->DISPCNT.screen_display_bg2) {
        render_bg_regular(ppu, &bgbuf[2], &ppu->BG2CNT, ppu->BG2HOFS.offset, ppu->BG2VOFS.offset,
                          ppu->WININ.win0_bg2_enable, ppu->WININ.win1_bg2_enable, ppu->WINOUT.outside_bg2_enable, ppu->WINOUT.obj_bg2_enable);
    }

    if (ppu->DISPCNT.screen_display_bg3) {
        render_bg_regular(ppu, &bgbuf[3], &ppu->BG3CNT, ppu->BG3HOFS.offset, ppu->BG3VOFS.offset,
                          ppu->WININ.win0_bg3_enable, ppu->WININ.win1_bg3_enable, ppu->WINOUT.outside_bg3_enable, ppu->WINOUT.obj_bg3_enable);
    }

    refresh_background_priorities(ppu);

    merge_bgs(ppu);
}

INLINE void render_line_mode1(gba_ppu_t* ppu) {
    render_obj(ppu);

    if (ppu->DISPCNT.screen_display_bg0) {
        render_bg_regular(ppu, &bgbuf[0], &ppu->BG0CNT, ppu->BG0HOFS.offset, ppu->BG0VOFS.offset,
                          ppu->WININ.win0_bg0_enable, ppu->WININ.win1_bg0_enable, ppu->WINOUT.outside_bg0_enable, ppu->WINOUT.obj_bg0_enable);
    }

    if (ppu->DISPCNT.screen_display_bg1) {
        render_bg_regular(ppu, &bgbuf[1], &ppu->BG1CNT, ppu->BG1HOFS.offset, ppu->BG1VOFS.offset,
                          ppu->WININ.win0_bg1_enable, ppu->WININ.win1_bg1_enable, ppu->WINOUT.outside_bg1_enable, ppu->WINOUT.obj_bg1_enable);
    }

    if (ppu->DISPCNT.screen_display_bg2) {
        render_bg_affine(ppu, &bgbuf[2], &ppu->BG2CNT,
                         ppu->WININ.win0_bg2_enable, ppu->WININ.win1_bg2_enable, ppu->WINOUT.outside_bg2_enable, ppu->WINOUT.obj_bg2_enable,
                         &ppu->BG2X, &ppu->BG2Y, &ppu->BG2PA, &ppu->BG2PB, &ppu->BG2PC, &ppu->BG2PD);
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

void ppu_step(gba_ppu_t* ppu) {
    // Update coords and set V/HBLANK flags
    ppu->x++;
    if (!ppu->DISPSTAT.hblank && is_hblank(ppu)) {
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
    if (ppu->x >= GBA_SCREEN_X + GBA_SCREEN_HBLANK) {
        ppu->x = 0;
        dbg_tick(SCANLINE);
        ppu->DISPSTAT.hblank = false;
        ppu->y++;

        if (ppu->y > GBA_SCREEN_Y + GBA_SCREEN_VBLANK) {
            ppu->y = 0;
            dbg_tick(FRAME);
            ppu->DISPSTAT.vblank = false;
        }

        if (!ppu->DISPSTAT.vblank && is_vblank(ppu)) {
            dma_start_trigger(VBlank);
            if (ppu->DISPSTAT.vblank_irq_enable) {
                request_interrupt(IRQ_VBLANK);
            }
            ppu->DISPSTAT.vblank = true;
            render_screen(&ppu->screen);
        }

        if (ppu->y == ppu->DISPSTAT.vcount_setting) {
            ppu->DISPSTAT.vcount = true;
            if (ppu->DISPSTAT.vcount_irq_enable) {
                request_interrupt(IRQ_VCOUNT);
            }
        } else {
            ppu->DISPSTAT.vcount = false;
        }

    }
}
