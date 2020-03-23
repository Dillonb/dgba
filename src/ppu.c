#include "ppu.h"
#include "common/log.h"
#include "gbabus.h"
#include "render.h"

gba_ppu_t* init_ppu() {
    gba_ppu_t* ppu = malloc(sizeof(gba_ppu_t));

    ppu->DISPCNT.raw = 0;

    ppu->x = 0;
    ppu->y = 0;

    return ppu;
}

bool is_hblank(gba_ppu_t* ppu) {
    return ppu->x > GBA_SCREEN_X;
}

bool is_vblank(gba_ppu_t* ppu) {
    return ppu->y > GBA_SCREEN_Y && ppu->y != 227;
}

bool is_visible(gba_ppu_t* ppu) {
    return ppu->x < GBA_SCREEN_X && ppu->y < GBA_SCREEN_Y;
}

#define PALETTE_BANK_BACKGROUND 0

void render_pixel_mode4(gba_ppu_t* ppu, color_t* pixel) {
    if (ppu->DISPCNT.screen_display_bg2) {
        int offset = ppu->x + (ppu->y * GBA_SCREEN_X); // Calculate this based on BG2X/Y/VOFS/HOFS/etc
        int index = ppu->DISPCNT.display_frame_select * 0xA000 + offset;
        int tile = ppu->vram[index];
        if (tile == 0) {
            pixel->a = 0;
            pixel->r = 0;
            pixel->g = 0;
            pixel->b = 0;
        } else {
            gba_color_t color;
            color.raw = gba_read_half(0x05000000 | (0x20 * PALETTE_BANK_BACKGROUND + 2 * tile)) & 0x7FFF;

            pixel->a = 0xFF;
            pixel->r = color.r << 3;
            pixel->g = color.g << 3;
            pixel->b = color.b << 3;
        }
    } else {
        pixel->a = 0;
        pixel->r = 0;
        pixel->g = 0;
        pixel->b = 0;
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

    if (is_visible(ppu) && !ppu->DISPCNT.forced_blank) {
        // Draw a pixel
        color_t* pixel = &ppu->screen[ppu->y][ppu->x];
        switch (ppu->DISPCNT.mode) {
            case 0:
                // logwarn("Ignoring render pixel in mode 0")
                pixel->a = 0;
                pixel->r = 0;
                pixel->g = 0;
                pixel->b = 0;
                break;
            case 4:
                render_pixel_mode4(ppu, pixel);
                break;
            default:
                logfatal("Unknown graphics mode: %d", ppu->DISPCNT.mode)
        }
    }
}
