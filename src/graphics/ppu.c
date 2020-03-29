#include "ppu.h"
#include "../common/log.h"
#include "../mem/gbabus.h"
#include "render.h"

gba_ppu_t* init_ppu() {
    gba_ppu_t* ppu = malloc(sizeof(gba_ppu_t));

    ppu->DISPCNT.raw = 0;
    ppu->DISPSTAT.raw = 0;

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

bool is_hblank(gba_ppu_t* ppu) {
    return ppu->x > GBA_SCREEN_X;
}

bool is_vblank(gba_ppu_t* ppu) {
    return ppu->y > GBA_SCREEN_Y && ppu->y != 227;
}

#define PALETTE_BANK_BACKGROUND 0

#define FIVEBIT_TO_EIGHTBIT_COLOR(c) (c<<3)|(c&7)

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

typedef union reg_se {
    half raw;
    struct {
        unsigned tid:10;
        bool hflip:1;
        bool vflip:1;
        unsigned pb:4;
    };
} reg_se_t;

#define SCREENBLOCK_SIZE 0x800
#define CHARBLOCK_SIZE  0x4000
void render_line_mode0(gba_ppu_t* ppu) {
    unimplemented(ppu->BG0CNT.is_256color == 1, "256 color mode")

    // Tileset (like pattern tables in the NES)
    word character_base_addr = 0x06000000 + ppu->BG0CNT.character_base_block * CHARBLOCK_SIZE;
    // Tile map (like nametables in the NES)
    word screen_base_addr = 0x06000000 + ppu->BG0CNT.screen_base_block * SCREENBLOCK_SIZE;

    int tile_size = ppu->BG0CNT.is_256color ? 0x40 : 0x20;
    int in_tile_offset_divisor = ppu->BG0CNT.is_256color ? 1 : 2;

    if (ppu->DISPCNT.screen_display_bg0) {
       //printf("screen_display_bg0 chr base: 0x%08X screen base: 0x%08X\n", ppu->BG0CNT.character_base_block, ppu->BG0CNT.screen_base_block);
    }
    unimplemented(ppu->DISPCNT.screen_display_bg1, "mode0 bg1")
    unimplemented(ppu->DISPCNT.screen_display_bg2, "mode0 bg2")
    unimplemented(ppu->DISPCNT.screen_display_bg3, "mode0 bg3")
    reg_se_t se;
    for (int x = 0; x < GBA_SCREEN_X; x++) {
        int screenblock_number;
        int tilemap_x;
        int tilemap_y;
        switch (ppu->BG0CNT.screen_size) {
            case 0:
                tilemap_x = (x + ppu->BG0HOFS.offset) % 256;
                tilemap_y = (ppu->y + ppu->BG0VOFS.offset) % 256;
                screenblock_number = 0;
                break;
            case 1:
                screenblock_number = ((x + ppu->BG0HOFS.offset) % 512) > 255 ? 1 : 0;
                tilemap_x = (x + ppu->BG0HOFS.offset) % 256;
                tilemap_y = (ppu->y + ppu->BG0VOFS.offset) % 256;
                break;
            default:
                logfatal("Unimplemented screen size: %d", ppu->BG0CNT.screen_size);

        }
        int se_number = (tilemap_x / 8) + (tilemap_y / 8) * 32;
        se.raw = gba_read_half(screen_base_addr + screenblock_number * SCREENBLOCK_SIZE + se_number * 2);

        // Find the tile
        word tile_address = character_base_addr + se.tid * tile_size;
        int tile_x = tilemap_x % 8;
        if (se.hflip) {
            tile_x = 7 - tile_x;
        }
        int tile_y = tilemap_y % 8;
        if (se.vflip) {
            tile_y = 7 - tile_y;
        }
        int in_tile_offset = tile_x + tile_y * 8;
        tile_address += in_tile_offset / in_tile_offset_divisor;

        byte tile = gba_read_byte(tile_address);

        if (!ppu->BG0CNT.is_256color) {
            tile >>= (in_tile_offset % 2) * 4;
            tile &= 0xF;
        }

        gba_color_t color;
        word palette_address = 0x05000000 + (0x20 * se.pb + 2 * tile);
        color.raw = gba_read_half(palette_address);

        ppu->screen[ppu->y][x].a = 0xFF;
        ppu->screen[ppu->y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(color.r);
        ppu->screen[ppu->y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(color.g);
        ppu->screen[ppu->y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(color.b);
    }
}

void render_line(gba_ppu_t* ppu) {
    // Draw a pixel
    switch (ppu->DISPCNT.mode) {
        case 0:
            render_line_mode0(ppu);
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
        ppu->DISPSTAT.hblank = false;
        ppu->y++;
        if (!ppu->DISPSTAT.vblank && is_vblank(ppu)) {
            if (ppu->DISPSTAT.vblank_irq_enable) {
                request_interrupt(IRQ_VBLANK);
            }
            ppu->DISPSTAT.vblank = true;
            render_screen(&ppu->screen);
        }

        if (ppu->y == ppu->DISPSTAT.vcount_setting) {
            ppu->DISPSTAT.vcount = true;
            logwarn("VCOUNT flag is set!!!")
            if (ppu->DISPSTAT.vcount_irq_enable) {
                request_interrupt(IRQ_VCOUNT);
            }
        } else {
            ppu->DISPSTAT.vcount = false;
        }

        if (ppu->y > GBA_SCREEN_Y + GBA_SCREEN_VBLANK) {
            ppu->y = 0;
            ppu->DISPSTAT.vblank = false;
        }
    }
}
