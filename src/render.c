#include <SDL2/SDL.h>

#include "render.h"
#include "common/log.h"
#include <stdbool.h>

#define SCREEN_SCALE 4

bool initialized = false;
SDL_Window* window = NULL;
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

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_STREAMING, GBA_SCREEN_X, GBA_SCREEN_Y);

    if (renderer == NULL) {
        logfatal("SDL couldn't create a renderer! %s", SDL_GetError());
    }

    SDL_RenderSetScale(renderer, SCREEN_SCALE, SCREEN_SCALE);
}

void update_key(SDL_Keycode sdlk, bool state) {
    switch (sdlk) {
        case SDLK_ESCAPE:
            logfatal("User pressed escape")
            break;
        default:
            logwarn("Unknown key: %d", sdlk)
            break;
    }
}

void render_screen(color_t (*screen)[GBA_SCREEN_X][GBA_SCREEN_Y]) {
    if (!initialized) {
        initialize();
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                logfatal("User requested quit");
            case SDL_KEYDOWN:
                update_key(event.key.keysym.sym, true);
                break;
            case SDL_KEYUP:
                update_key(event.key.keysym.sym, false);
                break;
            default:
                break;
        }
    }

    SDL_UpdateTexture(buffer, NULL, screen, GBA_SCREEN_X * 4);
    SDL_RenderCopy(renderer, buffer, NULL, NULL);
    loginfo("Updating renderer")
    SDL_RenderPresent(renderer);
}
