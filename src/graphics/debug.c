#include <stdbool.h>

#define DUI_IMPLEMENTATION
#include <DUI/DUI.h>

#include "debug.h"
#include "../common/log.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define SCREEN_SCALE 1

SDL_Window* dbg_window = NULL;
SDL_Renderer* dbg_renderer = NULL;
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
                                  WINDOW_WIDTH * SCREEN_SCALE,
                                  WINDOW_HEIGHT * SCREEN_SCALE,
                                  SDL_WINDOW_SHOWN);


    dbg_window_id = SDL_GetWindowID(dbg_window);

    dbg_renderer = SDL_CreateRenderer(dbg_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_RenderSetScale(dbg_renderer, SCREEN_SCALE, SCREEN_SCALE);

    DUI_Init(dbg_window);

    if (dbg_renderer == NULL) {
        logfatal("SDL couldn't create a renderer! %s", SDL_GetError());
    }

    dbg_window_visible = true;
}

void teardown_dbg_sdl_window() {
    DUI_Term();
    SDL_DestroyRenderer(dbg_renderer);
    SDL_DestroyWindow(dbg_window);
    dbg_window_visible = false;
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
            DUI_Panel(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 48);
            DUI_Print("Mode: ");

            DUI_MoveCursorRelative(6 * DUI_CHAR_SIZE, 0);

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

            DUI_MoveCursorRelative(-6 * DUI_CHAR_SIZE, 0);

            for (int r = 0; r < 16; r++) {
                DUI_Println("r%02d: %08Xh", r, get_register(cpu, r));
            }
        }

        if (DUI_Tab("Video Registers", TAB_VIDEO_REGISTERS, &tab_index)) {
            DUI_MoveCursor(8, 40);
            DUI_Panel(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 48);
            DUI_Println("TAB #2");
        }

        if (DUI_Tab("Tile Data", TAB_TILE_DATA, &tab_index)) {
            DUI_MoveCursor(8, 40);
            DUI_Panel(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 48);
            DUI_Println("TAB #3");
        }

        if (DUI_Tab("Tile Map", TAB_TILE_MAP, &tab_index)) {
            DUI_MoveCursor(8, 40);
            DUI_Panel(WINDOW_WIDTH - 16, WINDOW_HEIGHT - 48);
            DUI_Println("TAB #4");
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
    if (key == SDLK_p) {
        set_dbg_window_visibility(false);
    }
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
