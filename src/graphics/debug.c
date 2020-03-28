#include <stdbool.h>

#define DUI_IMPLEMENTATION
#include <DUI/DUI.h>

#include "debug.h"
#include "../common/log.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

SDL_Window* dbg_window = NULL;
SDL_Renderer* dbg_renderer = NULL;
SDL_Texture* dbg_buffer = NULL;
int dbg_window_id;

bool dbg_window_visible = false;

arm7tdmi_t* cpu;
gba_ppu_t* ppu;

void debug_init(arm7tdmi_t* new_cpu, gba_ppu_t* new_ppu) {
    cpu = new_cpu;
    ppu = new_ppu;
}

void setup_dbg_sdl_window() {
    dbg_window = SDL_CreateWindow("dgb dbg",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  WINDOW_WIDTH,
                                  WINDOW_HEIGHT,
                                  SDL_WINDOW_SHOWN);

    DUI_Init(dbg_window);

    dbg_window_id = SDL_GetWindowID(dbg_window);

    dbg_renderer = SDL_CreateRenderer(dbg_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    dbg_buffer = SDL_CreateTexture(dbg_renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH,
                                   WINDOW_HEIGHT);

    if (dbg_renderer == NULL) {
        logfatal("SDL couldn't create a renderer! %s", SDL_GetError());
    }

    dbg_window_visible = true;
}

void teardown_dbg_sdl_window() {
    SDL_DestroyTexture(dbg_buffer);
    SDL_DestroyRenderer(dbg_renderer);
    SDL_DestroyWindow(dbg_window);
    DUI_Term();
}

enum {
    TAB_CPU_REGISTERS,
    TAB_VIDEO_REGISTERS,
    TAB_TILE_DATA,
    TAB_TILE_MAP
};

int tab_index = TAB_CPU_REGISTERS;

void dbg_tick() {
    if (dbg_window_visible) {
        DUI_Update();
        SDL_SetRenderDrawColor(dbg_renderer, 0x33, 0x33, 0x33, 0xFF);
        SDL_RenderClear(dbg_renderer);
        DUI_MoveCursor(8, 8);

        DUI_BeginTabBar();

        if (DUI_Tab("CPU Registers", TAB_CPU_REGISTERS, &tab_index)) {
            DUI_MoveCursor(8, 40);
            DUI_Panel(800 - 16, 600 - 48);
        }

        if (DUI_Tab("Video Registers", TAB_VIDEO_REGISTERS, &tab_index)) {
            DUI_MoveCursor(8, 40);
            DUI_Panel(800 - 16, 600 - 48);
        }

        if (DUI_Tab("Tile Data", TAB_TILE_DATA, &tab_index)) {
            DUI_MoveCursor(8, 40);
            DUI_Panel(800 - 16, 600 - 48);
        }

        if (DUI_Tab("Tile Map", TAB_TILE_MAP, &tab_index)) {
            DUI_MoveCursor(8, 40);
            DUI_Panel(800 - 16, 600 - 48);
        }

        SDL_RenderPresent(dbg_renderer);
    }
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
