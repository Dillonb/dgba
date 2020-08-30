#include <SDL.h>
#include <SDL_gamecontroller.h>

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

// TODO: Support multiple, maybe up to 4?
SDL_GameController* controller = NULL;
SDL_Joystick* joystick = NULL;
int joystickId = -1;

void gba_refresh_gamepads() {
    if (controller != NULL) {
        SDL_GameControllerClose(controller);
        controller = NULL;
        joystick = NULL;
    }

    bool found_one = false;

    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            if (!found_one) {
                found_one = true;
                controller = SDL_GameControllerOpen(i);
                if (controller) {
                    joystick = SDL_GameControllerGetJoystick(controller);
                }
            } else {
                printf("Found more than one joystick! WARNING: only the first will be used!\n");
            }
        }
    }
}

void initialize() {
    initialized = true;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
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

void update_joybutton(byte button, bool state) {
    KEYINPUT_t* KEYINPUT = get_keyinput();
    switch (button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            KEYINPUT->up = !state;
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            KEYINPUT->down = !state;
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            KEYINPUT->left = !state;
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            KEYINPUT->right = !state;
            break;
        case SDL_CONTROLLER_BUTTON_A:
            KEYINPUT->a = !state;
            break;
        case SDL_CONTROLLER_BUTTON_B:
            KEYINPUT->b = !state;
            break;
        case SDL_CONTROLLER_BUTTON_START:
            KEYINPUT->start = !state;
            break;
        case SDL_CONTROLLER_BUTTON_GUIDE:
        case SDL_CONTROLLER_BUTTON_BACK:
            KEYINPUT->select = !state;
            break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            KEYINPUT->l = !state;
            break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            KEYINPUT->r = !state;
            break;
        default:
            break;
    }
}

int16_t joyx, joyy;

#define SLICE_OFFSET 67.5
#define CHECKSLICE(degrees, angle) (degrees > (angle - SLICE_OFFSET) && degrees < (angle + SLICE_OFFSET))

void update_joyaxis(byte axis, int16_t value) {
    KEYINPUT_t* KEYINPUT = get_keyinput();
    switch (axis) {
        case SDL_CONTROLLER_AXIS_LEFTX:
            joyx = value;
            break;
        case SDL_CONTROLLER_AXIS_LEFTY:
            joyy = value;
            break;
        default:
            break;
    }

    if (abs(joyx) < 8000 && abs(joyy) < 8000) {
        KEYINPUT->left = true;
        KEYINPUT->right = true;
        KEYINPUT->up = true;
        KEYINPUT->down = true;
    } else {

        // normalize to unit circle
        double adjusted_joyx = (double)joyx / 32768;
        double adjusted_joyy = (double)joyy / -32768; // y axis is reversed from what you'd expect, so flip it back with this division

        // what direction are we pointing?
        double degrees = atan2(adjusted_joyy, adjusted_joyx) * 180 / M_PI;

        // atan2 returns negative numbers for values > 180
        if (degrees < 0) {
            degrees = 360 + degrees;
        }

        // 135 degree slices, overlapping.
        KEYINPUT->up = !CHECKSLICE(degrees, 90);
        KEYINPUT->down = !CHECKSLICE(degrees, 270);
        KEYINPUT->left = !CHECKSLICE(degrees, 180);
        // Slightly different since it's around the 0 angle
        KEYINPUT->right = !(degrees < SLICE_OFFSET || degrees > (360 - SLICE_OFFSET));
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
            }
            break;
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMAPPED:
            gba_refresh_gamepads();
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            update_joybutton(event->cbutton.button, true);
            break;
        case SDL_CONTROLLERBUTTONUP:
            update_joybutton(event->cbutton.button, false);
            break;
        case SDL_CONTROLLERAXISMOTION:
            update_joyaxis(event->caxis.axis, event->caxis.value);
            break;
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
