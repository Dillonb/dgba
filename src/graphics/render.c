#include <SDL.h>

#include "render.h"
#include "../common/log.h"
#include "../mem/gbabus.h"
#include "debug.h"
#include "../gba_system.h"

static int SCREEN_SCALE = 4;

void set_screen_scale(int scale) {
    SCREEN_SCALE = scale;
}

static bool initialized = false;
static bool ctrl_state = false;
static SDL_Window* window = NULL;
static uint32_t window_id;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* buffer = NULL;

void initialize() {
    initialized = true;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        logfatal("SDL couldn't initialize! %s", SDL_GetError());
    }

    window = SDL_CreateWindow("dgb gba",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            GBA_SCREEN_X * SCREEN_SCALE,
            GBA_SCREEN_Y * SCREEN_SCALE,
            SDL_WINDOW_SHOWN);
    window_id = SDL_GetWindowID(window);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_STREAMING, GBA_SCREEN_X, GBA_SCREEN_Y);

    if (renderer == NULL) {
        logfatal("SDL couldn't create a renderer! %s", SDL_GetError());
    }

    SDL_RenderSetScale(renderer, SCREEN_SCALE, SCREEN_SCALE);
}

void save_load(bool state, int i) {
    if (state) {
        if (ctrl_state) {
            load_state(mem->savestate_path[i]);
        } else {
            save_state(mem->savestate_path[i]);
        }
    }
}

void update_key(SDL_Keycode sdlk, bool state) {
    KEYINPUT_t* KEYINPUT = get_keyinput();
    switch (sdlk) {
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            ctrl_state = state;
            break;
        case SDLK_ESCAPE:
            logwarn("User pressed escape")
            if (ctrl_state) {
                should_quit = true;
            }
            break;
        case SDLK_UP:
        case SDLK_w:
            KEYINPUT->up = !state;
            break;
        case SDLK_DOWN:
        case SDLK_s:
            KEYINPUT->down = !state;
            break;
        case SDLK_LEFT:
        case SDLK_a:
            KEYINPUT->left = !state;
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            KEYINPUT->right = !state;
            break;
        case SDLK_j:
            KEYINPUT->a = !state;
            break;
        case SDLK_k:
            KEYINPUT->b = !state;
            break;
        case SDLK_RETURN:
            KEYINPUT->start = !state;
            break;
        case SDLK_RSHIFT:
            KEYINPUT->select = !state;
            break;
        case SDLK_q:
            KEYINPUT->l = !state;
            break;
        case SDLK_e:
            KEYINPUT->r = !state;
            break;
        case SDLK_1:
            save_load(state, 0);
            break;
        case SDLK_2:
            save_load(state, 1);
            break;
        case SDLK_3:
            save_load(state, 2);
            break;
        case SDLK_4:
            save_load(state, 3);
            break;
        case SDLK_5:
            save_load(state, 4);
            break;
        case SDLK_6:
            save_load(state, 5);
            break;
        case SDLK_7:
            save_load(state, 6);
            break;
        case SDLK_8:
            save_load(state, 7);
            break;
        case SDLK_9:
            save_load(state, 8);
            break;
        case SDLK_0:
            save_load(state, 9);
            break;
        default:
            break;
    }
}

void gba_handle_event(SDL_Event* event) {
    switch (event->type) {
        case SDL_QUIT:
            logwarn("User requested quit")
            should_quit = true;
            break;
        case SDL_KEYDOWN:
            if (event->key.windowID == window_id) {
                if (event->key.keysym.sym == SDLK_o) {
                    set_dbg_window_visibility(true);
                } else {
                    update_key(event->key.keysym.sym, true);
                }
                break;
            }
        case SDL_KEYUP:
            if (event->key.windowID == window_id) {
                update_key(event->key.keysym.sym, false);
                break;
            }
        default:
            break;
    }
}

uint32_t fps_interval = 1000; // 1000ms = 1 sec
uint32_t sdl_lastframe = 0;
uint32_t sdl_numframes = 0;
uint32_t sdl_fps = 0;
char sdl_wintitle[16] = "dgb gba 00 FPS";

void render_screen(color_t (*screen)[GBA_SCREEN_Y][GBA_SCREEN_X]) {
    if (!ppu->enable_graphics) {
        return;
    }
    if (!initialized) {
        initialize();
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        debug_handle_event(&event);
        gba_handle_event(&event);
    }

    SDL_UpdateTexture(buffer, NULL, screen, GBA_SCREEN_X * 4);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);
    loginfo("Updating renderer")
    SDL_RenderPresent(renderer);
    sdl_numframes++;
    uint32_t ticks = SDL_GetTicks();
    if (sdl_lastframe < ticks - fps_interval) {
        sdl_lastframe = ticks;
        sdl_fps = sdl_numframes;
        sdl_numframes = 0;
        snprintf(sdl_wintitle, sizeof(sdl_wintitle), "dgb gba %02d FPS", sdl_fps);
        SDL_SetWindowTitle(window, sdl_wintitle);
    }
    for (int y = 0; y < GBA_SCREEN_Y; y++) {
        for (int x = 0; x < GBA_SCREEN_X; x++) {
            (*screen)[y][x].r = 0;
            (*screen)[y][x].g = 0;
            (*screen)[y][x].b = 0;
        }
    }
}
