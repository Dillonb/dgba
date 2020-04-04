#include <SDL.h>

#include "render.h"
#include "../common/log.h"
#include "../mem/gbabus.h"
#include "debug.h"
#include <stdbool.h>

#define SCREEN_SCALE 4

bool initialized = false;
SDL_Window* window = NULL;
uint32_t window_id;
SDL_Renderer* renderer = NULL;
SDL_Texture* buffer = NULL;

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

void update_key(SDL_Keycode sdlk, bool state) {
    KEYINPUT_t* KEYINPUT = get_keyinput();
    switch (sdlk) {
        case SDLK_ESCAPE:
            logfatal("User pressed escape")
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
        default:
            break;
    }
}

void gba_handle_event(SDL_Event* event) {
    switch (event->type) {
        case SDL_QUIT:
            logfatal("User requested quit");
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

void render_screen(color_t (*screen)[GBA_SCREEN_Y][GBA_SCREEN_X]) {
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
    for (int y = 0; y < GBA_SCREEN_Y; y++) {
        for (int x = 0; x < GBA_SCREEN_X; x++) {
            (*screen)[y][x].r = 0;
            (*screen)[y][x].g = 0;
            (*screen)[y][x].b = 0;
        }
    }
}
