#include <stdbool.h>

#define DUI_IMPLEMENTATION
#include <DUI/DUI.h>

#include "debug.h"
#include "../common/log.h"
#include "render.h"
#include "../gba_system.h"

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 1050

#define SCREEN_SCALE 1

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static int dbg_window_id;

static SDL_Texture* dbg_tilemap_texture = NULL;
static color_t dbg_tilemap[256][256];
static int dbg_tilemap_pb = -1;

bool dbg_window_visible = false;

dbg_tick_t tick_on = INSTRUCTION;

void setup_dbg_sdl_window() {
    window = SDL_CreateWindow("dgb gba dbg",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                                  WINDOW_WIDTH * SCREEN_SCALE,
                                  WINDOW_HEIGHT * SCREEN_SCALE,
                              SDL_WINDOW_SHOWN);


    dbg_window_id = SDL_GetWindowID(window);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    dbg_tilemap_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_STREAMING, 256, 256);

    SDL_RenderSetScale(renderer, SCREEN_SCALE, SCREEN_SCALE);

    DUI_Init(window);

    DUI_Style * style = DUI_GetStyle();
    style->CharSize = 8 * SCREEN_SCALE;
    style->LineHeight = 10 * SCREEN_SCALE;

    if (renderer == NULL) {
        logfatal("SDL couldn't create a renderer! %s", SDL_GetError());
    }

    dbg_window_visible = true;
}

void teardown_dbg_sdl_window() {
    DUI_Term();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    dbg_window_visible = false;
}

enum {
    TAB_CPU_REGISTERS,
    TAB_VIDEO_REGISTERS,
    TAB_TILE_DATA,
    TAB_TILE_MAP
};

int tab_index = TAB_CPU_REGISTERS;

#define cpsrflag(f, c) (f == 1?c:"-")
#define printcpsr(cpsr) DUI_Println("CPSR: %08Xh [%s%s%s%s%s%s%s]", cpsr.raw, cpsrflag(cpsr.N, "N"), cpsrflag(cpsr.Z, "Z"), \
         cpsrflag(cpsr.C, "C"), cpsrflag(cpsr.V, "V"), cpsrflag(cpsr.disable_irq, "I"), \
         cpsrflag(cpsr.disable_fiq, "F"), cpsrflag(cpsr.thumb, "T"))

#define print_bgcnt(n, reg) DUI_Println("BG%dCNT: %08Xh\n " \
            "priority: %d, chr_base: %d, mosiac: %d, 256color: %d, scr_base: %d, screen_size: %d\n\n" \
            , n, \
            reg.raw, \
            reg.priority, \
            reg.character_base_block, \
            reg.mosaic, \
            reg.is_256color, \
            reg.screen_base_block, \
            reg.screen_size)

#define print_bgofs(n, hofs, vofs) DUI_Println("BG%dOFS: %08Xh / %08Xh", n, hofs.raw, vofs.raw)

void print_timer(int n, TMCNT_H_t* tmcnth, int timer_reload, TMINT_t* tmint) {
    const char* prescaler_selection = "UNKNOWN";
    switch (tmcnth->frequency) {
        case 0:
            prescaler_selection = "F/1";
            break;
        case 1:
            prescaler_selection = "F/64";
            break;
        case 2:
            prescaler_selection = "F/256";
            break;
        case 3:
            prescaler_selection = "F/1024";
            break;
    }
    const char* enabled = tmcnth->start ? "ENABLED" : "disabled";
    DUI_Println("\nTimer %d: %s", n, enabled);
    const char* tm0msg = n == 0 ? " (unused in Timer 0)" : "";
    DUI_Println(" Reload: 0x%04X\n"
                " Prescaler: %s\n"
                " Count-Up Timing:%s %d\n"
                " IRQ: %d\n"
                " Value: 0x%04X\n",
                timer_reload,
                prescaler_selection,
                tm0msg,
                tmcnth->cascade,
                tmcnth->timer_irq_enable,
                tmint->value);
}

void print_dma(int d, DMACNTH_t *cnth, unsigned int wc, DMAINT_t *dmaint, unsigned int sad, unsigned int dad) {
    DUI_Println("%d: enable: %d, start time: %d irq: %d sadctrl: %d dadctrl: %d",
                d,
                cnth->dma_enable,
                cnth->dma_start_time,
                cnth->irq_on_end_of_wc,
                cnth->source_addr_control,
                cnth->dest_addr_control);
}


void ramdump(word base_address, word size) {
    bool allzeroes = true;
    for (int x = 0; x < size; x += 0x10) {
        half a = gba_read_half(base_address + x);
        half b = gba_read_half(base_address + x + 0x2);
        half c = gba_read_half(base_address + x + 0x4);
        half d = gba_read_half(base_address + x + 0x6);
        half e = gba_read_half(base_address + x + 0x8);
        half f = gba_read_half(base_address + x + 0xA);
        half g = gba_read_half(base_address + x + 0xC);
        half h = gba_read_half(base_address + x + 0xE);
        if (a != 0 || b != 0 || c != 0 || d != 0 || e != 0 || f != 0 || g != 0 || h != 0) {
            allzeroes = false;
        }
        printf("%08X: %04X %04X %04X %04X %04X %04X %04X %04X\n",
               base_address + x, a, b, c, d, e, f, g, h);
    }
    if (allzeroes) {
        printf("Warning: all zeroes!\n");
    }
}

void print_bgparam(int bg, bg_referencepoint_t x, bg_referencepoint_t y, bg_rotation_scaling_t pa, bg_rotation_scaling_t pb, bg_rotation_scaling_t pc, bg_rotation_scaling_t pd) {
    DUI_Println("BG%d - X: %s%d.%d Y: %s%d.%d PA: %s%d.%d PB: %s%d.%d PC: %s%d.%d PD: %s%d.%d", bg,
                x.sign ? "-" : "+", x.integer, x.fractional,
                y.sign ? "-" : "+", y.integer, y.fractional,
                pa.sign ? "-" : "+", pa.integer, pa.fractional,
                pb.sign ? "-" : "+", pb.integer, pb.fractional,
                pc.sign ? "-" : "+", pc.integer, pc.fractional,
                pd.sign ? "-" : "+", pd.integer, pd.fractional);
}

void actual_dbg_tick() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        debug_handle_event(&event);
        gba_handle_event(&event);
    }
    DUI_Update();
    SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0xFF);
    SDL_RenderClear(renderer);
    DUI_MoveCursor(8, 8);

    DUI_BeginTabBar();

    if (DUI_Tab("CPU Registers", TAB_CPU_REGISTERS, &tab_index)) {
        DUI_MoveCursor(8, 40);
        DUI_Panel(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 48);
        DUI_Println("IF: %08Xh", bus->IF.raw);
        DUI_Println("Halt: %d\nIRQ: %d", cpu->halt, cpu->irq);
        DUI_Print("Mode: ");

        const char* execution_mode = cpu->cpsr.thumb ? "[THM]" : "[ARM]";

        switch (cpu->cpsr.mode) {
            case MODE_USER:
                DUI_Println("User %s", execution_mode);
                break;
            case MODE_FIQ:
                DUI_Println("FIQ %s", execution_mode);
                break;
            case MODE_SUPERVISOR:
                DUI_Println("Supervisor %s", execution_mode);
                break;
            case MODE_ABORT:
                DUI_Println("Abort %s", execution_mode);
                break;
            case MODE_IRQ:
                DUI_Println("IRQ %s", execution_mode);
                break;
            case MODE_UNDEFINED:
                DUI_Println("Undefined %s", execution_mode);
                break;
            case MODE_SYSTEM:
                DUI_Println("System %s", execution_mode);
                break;
        }

        printcpsr(cpu->cpsr);

        for (int r = 0; r < 16; r++) {
            DUI_Println("r%02d:  %08Xh", r, get_register(cpu, r));
        }

        DUI_Println("\n--- Timers ---");

        for (int t = 0; t < 4; t++) {
            print_timer(t, &bus->TMCNT_H[t], bus->TMCNT_L[t].timer_reload, &(bus->TMINT[t]));
        }

        DUI_Println("\n--- DMAs ---");

        print_dma(0, &bus->DMA0CNT_H, bus->DMA0CNT_L.wc, &bus->DMA0INT, bus->DMA0SAD.addr, bus->DMA0DAD.addr);
        print_dma(1, &bus->DMA1CNT_H, bus->DMA1CNT_L.wc, &bus->DMA1INT, bus->DMA1SAD.addr, bus->DMA1DAD.addr);
        print_dma(2, &bus->DMA2CNT_H, bus->DMA2CNT_L.wc, &bus->DMA2INT, bus->DMA2SAD.addr, bus->DMA2DAD.addr);
        print_dma(3, &bus->DMA3CNT_H, bus->DMA3CNT_L.wc, &bus->DMA3INT, bus->DMA3SAD.addr, bus->DMA3DAD.addr);

        DUI_Println("KEYINPUT: 0x%04X", bus->KEYINPUT.raw);
    }

    if (DUI_Tab("Video Registers", TAB_VIDEO_REGISTERS, &tab_index)) {
        DUI_MoveCursor(8, 40);
        DUI_Panel(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 48);

        DUI_Println("DISPSTAT: 0x%08Xh\n vbl: %d\n hbl: %d\n vcnt %d\n IRQ:\n  vbl: %d\n  hbl: %d\n  vcount: %d\n  vcount_setting: %02Xh\n",
                    ppu->DISPSTAT.raw,
                    ppu->DISPSTAT.vblank,
                    ppu->DISPSTAT.hblank,
                    ppu->DISPSTAT.vcount,
                    ppu->DISPSTAT.vblank_irq_enable,
                    ppu->DISPSTAT.hblank_irq_enable,
                    ppu->DISPSTAT.vcount_irq_enable,
                    ppu->DISPSTAT.vcount_setting);

        DUI_Println("DISPCNT: %08Xh\n mode: %d\n cgbmode: %d\n display_frame_select: %d\n hblank_interval_free: %d\n obj_character_vram_mapping: %s\n"
                    " forced_blank: %d\n display: bg0: %d bg1: %d bg2: %d bg3: %d win0: %d win1: %d objwin %d]\n\n",
                    ppu->DISPCNT.raw,
                    ppu->DISPCNT.mode,
                    ppu->DISPCNT.cgbmode,
                    ppu->DISPCNT.display_frame_select,
                    ppu->DISPCNT.hblank_interval_free,
                    ppu->DISPCNT.obj_character_vram_mapping ? "1D" : "2D",
                    ppu->DISPCNT.forced_blank,
                    ppu->DISPCNT.screen_display_bg0,
                    ppu->DISPCNT.screen_display_bg1,
                    ppu->DISPCNT.screen_display_bg2,
                    ppu->DISPCNT.screen_display_bg3,
                    ppu->DISPCNT.window0_display,
                    ppu->DISPCNT.window1_display,
                    ppu->DISPCNT.obj_window_display);

        print_bgcnt(0, ppu->BG0CNT);
        print_bgcnt(1, ppu->BG1CNT);
        print_bgcnt(2, ppu->BG2CNT);
        print_bgcnt(3, ppu->BG3CNT);

        print_bgofs(0, ppu->BG0HOFS, ppu->BG0VOFS);
        print_bgofs(1, ppu->BG1HOFS, ppu->BG1VOFS);
        print_bgofs(2, ppu->BG2HOFS, ppu->BG2VOFS);
        print_bgofs(3, ppu->BG3HOFS, ppu->BG3VOFS);

        print_bgparam(2, ppu->BG2X, ppu->BG2Y, ppu->BG2PA, ppu->BG2PB, ppu->BG2PC, ppu->BG2PD);
        print_bgparam(3, ppu->BG3X, ppu->BG3Y, ppu->BG3PA, ppu->BG3PB, ppu->BG3PC, ppu->BG3PD);
        DUI_Println("");
        DUI_Println("WIN0H x1: %d x2: %d", ppu->WIN0H.x1, ppu->WIN0H.x2);
        DUI_Println("WIN0V y1: %d y2: %d", ppu->WIN0V.y1, ppu->WIN0V.y2);
        DUI_Println("WIN1H x1: %d x2: %d", ppu->WIN1H.x1, ppu->WIN1H.x2);
        DUI_Println("WIN1V y1: %d y2: %d", ppu->WIN1V.y1, ppu->WIN1V.y2);
        DUI_Println("");

        DUI_Println("WIN0   bg0: %d bg1: %d bg2: %d bg3: %d\n"
                    "WIN1   bg0: %d bg1: %d bg2: %d bg3: %d\n"
                    "WINOUT bg0: %d bg1: %d bg2: %d bg3: %d",
                    ppu->WININ.win0_bg0_enable, ppu->WININ.win0_bg1_enable, ppu->WININ.win0_bg2_enable, ppu->WININ.win0_bg3_enable,
                    ppu->WININ.win1_bg0_enable, ppu->WININ.win1_bg1_enable, ppu->WININ.win1_bg2_enable, ppu->WININ.win1_bg3_enable,
                    ppu->WINOUT.outside_bg0_enable, ppu->WINOUT.outside_bg1_enable, ppu->WINOUT.outside_bg2_enable, ppu->WINOUT.outside_bg3_enable);
    }

    if (DUI_Tab("RAM Dumping", TAB_TILE_DATA, &tab_index)) {
        DUI_MoveCursor(8, 40);
        DUI_Panel(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 48);

        if (DUI_Button("Dump EWRAM")) {
            ramdump(0x02000000, EWRAM_SIZE);
        }

        if (DUI_Button("Dump IWRAM")) {
            ramdump(0x03000000, IWRAM_SIZE);
        }

        if (DUI_Button("Dump PRAM")) {
            ramdump(0x05000000, PRAM_SIZE);
        }

        if (DUI_Button("Dump VRAM")) {
            ramdump(0x06000000, VRAM_SIZE);
        }

        if (DUI_Button("Dump OAM")) {
            ramdump(0x07000000, OAM_SIZE);
        }
    }

    if (DUI_Tab("Tile Map", TAB_TILE_MAP, &tab_index)) {
        DUI_MoveCursor(8, 40);
        DUI_Panel(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 48);
        if (dbg_tilemap_pb < 0) {
            dbg_tilemap_pb = -1;
            DUI_Println("OBJ Tilemap PB: AUTO");
        } else {
            DUI_Println("OBJ Tilemap PB: %d", dbg_tilemap_pb);
        }

        if (DUI_Button("PB -1")) {
            dbg_tilemap_pb--;
        }

        if (DUI_Button("PB +1")) {
            dbg_tilemap_pb++;
        }
        obj_attr0_t attr0;
        obj_attr1_t attr1;
        obj_attr2_t attr2;

        int tile_pbs[32][32];

        for (int x = 0; x < 32; x++) {
            for (int y = 0; y < 32; y++) {
                tile_pbs[y][x] = 0;
            }
        }

        for (int sprite = 127; sprite >= 0; sprite--) {
            attr0.raw = gba_read_half(0x07000000 + (sprite * 8) + 0);
            attr1.raw = gba_read_half(0x07000000 + (sprite * 8) + 2);
            attr2.raw = gba_read_half(0x07000000 + (sprite * 8) + 4);

            int height = sprite_heights[attr0.shape][attr1.size] / 8;
            int width = sprite_widths[attr0.shape][attr1.size] / 8;

            int base_tid = attr2.tid;
            int base_tid_x = base_tid % 32;
            int base_tid_y = base_tid / 32;

            if (ppu->DISPCNT.obj_character_vram_mapping) { // 2D
                for (int sprite_x = 0; sprite_x < width; sprite_x++) {
                    for (int sprite_y = 0; sprite_y < height; sprite_y++) {
                        int ofs = base_tid + sprite_x + sprite_y * width;
                        int x = ofs % 32;
                        int y = ofs / 32;
                        tile_pbs[x][y] = attr2.pb;
                    }
                }
            } else {
                for (int x = base_tid_x; x < base_tid_x + width; x++) {
                    for (int y = base_tid_y; y < base_tid_y + height; y++) {
                        tile_pbs[x][y] = attr2.pb;
                    }
                }

            }
        }

        for (int y = 0; y < 256; y++) {
            int tile_y = y / 8;
            int in_tile_y = y % 8;
            int tile_y_offset = tile_y * 32;
            for (int x = 0; x < 256; x++) {
                int tile_x = x / 8;
                int in_tile_x = x % 8;

                int tid = tile_y_offset + tile_x;

                word tile_address = 0x06010000 + tid * 0x20; // 0x20 = obj tile size
                int in_tile_offset = in_tile_x + in_tile_y * 8;
                if (!attr0.is_256color) {
                    in_tile_offset /= 2;
                }

                tile_address += in_tile_offset;

                byte tile = gba_read_byte(tile_address);

                if (!attr0.is_256color) {
                    tile >>= (in_tile_offset % 2) * 4;
                    tile &= 0xF;
                }

                word palette_address = 0x05000200; // OBJ palette base
                if (attr0.is_256color) {
                    palette_address += 2 * tile;
                } else {
                    int pb = dbg_tilemap_pb >= 0 ? dbg_tilemap_pb : tile_pbs[tile_x][tile_y];
                    palette_address += (0x20 * pb + 2 * tile);
                }

                gba_color_t color;
                color.raw = gba_read_half(palette_address);

                dbg_tilemap[y][x].a = 0xFF;
                dbg_tilemap[y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(color.r);
                dbg_tilemap[y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(color.g);
                dbg_tilemap[y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(color.b);
            }
        }

        SDL_UpdateTexture(dbg_tilemap_texture, NULL, dbg_tilemap, 256 * 4);
        SDL_Rect destRect;
        DUI_GetCursor(&destRect.x, &destRect.y);
        destRect.w = 768;
        destRect.h = 768;
        DUI_MoveCursorRelative(0, 1024);
        SDL_RenderCopy(renderer, dbg_tilemap_texture, NULL, &destRect);
    }

    DUI_MoveCursor(16, WINDOW_HEIGHT - 30);
    DUI_Print("Tick debugger every: ");
    if (DUI_Radio("Instruction", INSTRUCTION, (int*) &tick_on)) {
        tick_on = INSTRUCTION;
    }
    if (DUI_Radio("Scanline", SCANLINE, (int*) &tick_on)) {
        tick_on = SCANLINE;
    }
    if (DUI_Radio("Frame", FRAME, (int*) &tick_on)) {
        tick_on = FRAME;
    }

    SDL_RenderPresent(renderer);
}

void set_dbg_window_visibility(bool visible) {
    if (visible && !dbg_window_visible) {
        setup_dbg_sdl_window();
    } else if (!visible & dbg_window_visible) {
        teardown_dbg_sdl_window();
    }
}

void handle_keydown(SDL_Keycode key) {
}

void handle_keyup(SDL_Keycode key) {
    if (key == SDLK_p || key == SDLK_ESCAPE) {
        set_dbg_window_visibility(false);
    }
}

void debug_handle_event(SDL_Event* event) {
    switch (event->type) {
        case SDL_KEYDOWN:
            if (event->key.windowID == dbg_window_id) {
                handle_keydown(event->key.keysym.sym);
            }
            break;
        case SDL_KEYUP:
            if (event->key.windowID == dbg_window_id) {
                handle_keyup(event->key.keysym.sym);
            }
            break;
        case SDL_MOUSEMOTION:
            if (event->key.windowID == dbg_window_id) {

            }
        default:
            break;
    }
}
