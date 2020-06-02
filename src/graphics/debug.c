#include <stdbool.h>

#define DUI_IMPLEMENTATION
#include <DUI/DUI.h>

#include "debug.h"
#include "render.h"
#include "../gba_system.h"

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 1050

#define SCREEN_SCALE 1

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static int dbg_window_id;

static SDL_Texture* dbg_tilemap_texture = NULL;
static color_t dbg_tilemap[5][GBA_SCREEN_Y][GBA_SCREEN_X];

bool dbg_window_visible = false;

typedef enum dbg_layer {
    BG0,
    BG1,
    BG2,
    BG3,
    OBJ
} dbg_layer_t;

dbg_tick_t tick_on = FRAME;
dbg_layer_t display_layer = BG0;

void setup_dbg_sdl_window() {
    window = SDL_CreateWindow("dgb gba dbg",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                                  WINDOW_WIDTH * SCREEN_SCALE,
                                  WINDOW_HEIGHT * SCREEN_SCALE,
                              SDL_WINDOW_SHOWN);


    dbg_window_id = SDL_GetWindowID(window);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    dbg_tilemap_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_STREAMING, GBA_SCREEN_X, GBA_SCREEN_Y);

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
        half a = gba_read_half(base_address + x, ACCESS_UNKNOWN);
        half b = gba_read_half(base_address + x + 0x2, ACCESS_UNKNOWN);
        half c = gba_read_half(base_address + x + 0x4, ACCESS_UNKNOWN);
        half d = gba_read_half(base_address + x + 0x6, ACCESS_UNKNOWN);
        half e = gba_read_half(base_address + x + 0x8, ACCESS_UNKNOWN);
        half f = gba_read_half(base_address + x + 0xA, ACCESS_UNKNOWN);
        half g = gba_read_half(base_address + x + 0xC, ACCESS_UNKNOWN);
        half h = gba_read_half(base_address + x + 0xE, ACCESS_UNKNOWN);
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

void print_bgparam(int bg, bg_referencepoint_container_t x, bg_referencepoint_container_t y, bg_rotation_scaling_t pa, bg_rotation_scaling_t pb, bg_rotation_scaling_t pc, bg_rotation_scaling_t pd) {
    DUI_Println("BG%d - Initial X: %f Y: %f Current: X: %f Y: %f | PA: %f PB: %f PC: %f PD: %f", bg,
            REF_TO_DOUBLE(x.initial),
            REF_TO_DOUBLE(y.initial),
            REF_TO_DOUBLE(x.current),
            REF_TO_DOUBLE(y.current),
            ROTSCALE_TO_DOUBLE(&pa),
            ROTSCALE_TO_DOUBLE(&pb),
            ROTSCALE_TO_DOUBLE(&pc),
            ROTSCALE_TO_DOUBLE(&pd));
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

        DUI_Println("\n--- Interrupts ---");
        DUI_Println("                      IE IF");
        DUI_Println("lcd_vblank:           %d  %d", bus->interrupt_enable.lcd_vblank, bus->IF.vblank);
        DUI_Println("lcd_hblank:           %d  %d", bus->interrupt_enable.lcd_hblank, bus->IF.hblank);
        DUI_Println("lcd_vcounter_match:   %d  %d", bus->interrupt_enable.lcd_vcounter_match, bus->IF.vcount);
        DUI_Println("timer0_overflow:      %d  %d", bus->interrupt_enable.timer0_overflow, bus->IF.timer0);
        DUI_Println("timer1_overflow:      %d  %d", bus->interrupt_enable.timer1_overflow, bus->IF.timer1);
        DUI_Println("timer2_overflow:      %d  %d", bus->interrupt_enable.timer2_overflow, bus->IF.timer2);
        DUI_Println("timer3_overflow:      %d  %d", bus->interrupt_enable.timer3_overflow, bus->IF.timer3);
        DUI_Println("serial_communication: %d  %d", bus->interrupt_enable.serial_communication, bus->IF.sio);
        DUI_Println("dma_0:                %d  %d", bus->interrupt_enable.dma_0, bus->IF.dma0);
        DUI_Println("dma_1:                %d  %d", bus->interrupt_enable.dma_1, bus->IF.dma1);
        DUI_Println("dma_2:                %d  %d", bus->interrupt_enable.dma_2, bus->IF.dma2);
        DUI_Println("dma_3:                %d  %d", bus->interrupt_enable.dma_3, bus->IF.dma3);
        DUI_Println("keypad:               %d  %d", bus->interrupt_enable.keypad, bus->IF.keypad);
        DUI_Println("gamepak:              %d  %d", bus->interrupt_enable.gamepak, bus->IF.gamepak);

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

        DUI_Println("WIN0   bg0: %d bg1: %d bg2: %d bg3: %d obj: %d blend: %d\n"
                    "WIN1   bg0: %d bg1: %d bg2: %d bg3: %d obj: %d blend: %d\n"
                    "WINOBJ bg0: %d bg1: %d bg2: %d bg3: %d obj: %d blend: %d\n"
                    "WINOUT bg0: %d bg1: %d bg2: %d bg3: %d obj: %d blend: %d",
                    ppu->WININ.win0_bg0_enable, ppu->WININ.win0_bg1_enable, ppu->WININ.win0_bg2_enable, ppu->WININ.win0_bg3_enable, ppu->WININ.win0_obj_enable, ppu->WININ.win0_color_special_effect_enable,
                    ppu->WININ.win1_bg0_enable, ppu->WININ.win1_bg1_enable, ppu->WININ.win1_bg2_enable, ppu->WININ.win1_bg3_enable, ppu->WININ.win1_obj_enable, ppu->WININ.win1_color_special_effect_enable,
                    ppu->WINOUT.obj_bg0_enable, ppu->WINOUT.obj_bg1_enable, ppu->WINOUT.obj_bg2_enable, ppu->WINOUT.obj_bg3_enable, ppu->WINOUT.obj_obj_enable, ppu->WINOUT.obj_color_special_effect_enable,
                    ppu->WINOUT.outside_bg0_enable, ppu->WINOUT.outside_bg1_enable, ppu->WINOUT.outside_bg2_enable, ppu->WINOUT.outside_bg3_enable, ppu->WINOUT.outside_obj_enable, ppu->WINOUT.outside_color_special_effect_enable);

        DUI_Println("--- BLDCNT ---\n"
                    "blend mode: %d\n"
                    "top: bg0: %d bg1: %d bg2: %d bg3: %d obj: %d bd: %d\n"
                    "bot: bg0: %d bg1: %d bg2: %d bg3: %d obj: %d bd: %d\n",
                    ppu->BLDCNT.blend_mode,
                    ppu->BLDCNT.aBG0, ppu->BLDCNT.aBG1, ppu->BLDCNT.aBG2, ppu->BLDCNT.aBG3, ppu->BLDCNT.aOBJ, ppu->BLDCNT.aBD,
                    ppu->BLDCNT.bBG0, ppu->BLDCNT.bBG1, ppu->BLDCNT.bBG2, ppu->BLDCNT.bBG3, ppu->BLDCNT.bOBJ, ppu->BLDCNT.bBD);
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

        DUI_Print("Display: ");
        if (DUI_Radio("BG0", BG0, (int*) &display_layer)) {
            display_layer = BG0;
        }
        if (DUI_Radio("BG1", BG1, (int*) &display_layer)) {
            display_layer = BG1;
        }
        if (DUI_Radio("BG2", BG2, (int*) &display_layer)) {
            display_layer = BG2;
        }
        if (DUI_Radio("BG3", BG3, (int*) &display_layer)) {
            display_layer = BG3;
        }
        if (DUI_Radio("OBJ", OBJ, (int*) &display_layer)) {
            display_layer = OBJ;
        }
        DUI_MoveCursor(8, 80);
        DUI_Panel(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 48);

        SDL_UpdateTexture(dbg_tilemap_texture, NULL, dbg_tilemap[display_layer], GBA_SCREEN_X * 4);
        SDL_Rect destRect;
        DUI_GetCursor(&destRect.x, &destRect.y);
        destRect.w = GBA_SCREEN_X * 4;
        destRect.h = GBA_SCREEN_Y * 4;
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

void copy_texture_line(dbg_layer_t layer) {
    if (layer == OBJ) {
        for (int x = 0; x < GBA_SCREEN_X; x++) {
            dbg_tilemap[layer][ppu->y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(ppu->objbuf[x].r);
            dbg_tilemap[layer][ppu->y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(ppu->objbuf[x].g);
            dbg_tilemap[layer][ppu->y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(ppu->objbuf[x].b);
            dbg_tilemap[layer][ppu->y][x].a = 0xFF;
        }
    } else {
        for (int x = 0; x < GBA_SCREEN_X; x++) {
            dbg_tilemap[layer][ppu->y][x].r = FIVEBIT_TO_EIGHTBIT_COLOR(ppu->bgbuf[layer][x].r);
            dbg_tilemap[layer][ppu->y][x].g = FIVEBIT_TO_EIGHTBIT_COLOR(ppu->bgbuf[layer][x].g);
            dbg_tilemap[layer][ppu->y][x].b = FIVEBIT_TO_EIGHTBIT_COLOR(ppu->bgbuf[layer][x].b);
            dbg_tilemap[layer][ppu->y][x].a = 0xFF;
        }
    }
}

void dbg_line_drawn() {
    if (dbg_window_visible) {
        copy_texture_line(BG0);
        copy_texture_line(BG1);
        copy_texture_line(BG2);
        copy_texture_line(BG3);
        copy_texture_line(OBJ);
    }
}