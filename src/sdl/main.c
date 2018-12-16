/**
 * This file is part of Portmino.
 * 
 * Portmino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Portmino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Portmino.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "event.h"
#include "game.h"
#include "softrender.h"

static SDL_Window* g_window;
static SDL_Renderer* g_renderer;
static SDL_Texture* g_texture;

static event_t sdl_scancode_to_event(int code) {
    switch (code) {
    case SDL_SCANCODE_LEFT: return EVENT_LEFT;
    case SDL_SCANCODE_RIGHT: return EVENT_RIGHT;
    case SDL_SCANCODE_DOWN: return EVENT_SOFTDROP;
    case SDL_SCANCODE_UP: return EVENT_HARDDROP;
    case SDL_SCANCODE_Z: return EVENT_CCW;
    case SDL_SCANCODE_X: return EVENT_CW;
    default: return EVENT_NONE;
    }
}

static ievent_t sdl_scancode_to_ievent(int code) {
    switch (code) {
    case SDL_SCANCODE_ESCAPE: return IEVENT_PAUSE;
    case SDL_SCANCODE_F10: return IEVENT_RESTART;
    case SDL_SCANCODE_F11: return IEVENT_MAINMENU;
    case SDL_SCANCODE_F12: return IEVENT_QUIT;
    default: return IEVENT_NONE;
    }
}

static mevent_t sdl_scancode_to_mevent(int code) {
    switch (code) {
    case SDL_SCANCODE_UP: return MEVENT_UP;
    case SDL_SCANCODE_DOWN: return MEVENT_DOWN;
    case SDL_SCANCODE_LEFT: return MEVENT_LEFT;
    case SDL_SCANCODE_RIGHT: return MEVENT_RIGHT;
    case SDL_SCANCODE_RETURN: return MEVENT_OK;
    case SDL_SCANCODE_BACKSPACE: return MEVENT_CANCEL;
    default: return MEVENT_NONE;
    }
}

static void sdl_run(void) {
    // Assemble our game events from polled events.
    SDL_Event event;
    static gameinputs_t inputs = { 0 }; // keep the inputs around

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            if (event.key.repeat != 0) {
                break;
            }
            inputs.game |= sdl_scancode_to_event(event.key.keysym.scancode);
            inputs.interface |= sdl_scancode_to_ievent(event.key.keysym.scancode);
            inputs.menu |= sdl_scancode_to_mevent(event.key.keysym.scancode);
            break;
        case SDL_KEYUP:
            if (event.key.repeat != 0) {
                break;
            }
            inputs.game &= ~(sdl_scancode_to_event(event.key.keysym.scancode));
            inputs.interface &= ~(sdl_scancode_to_ievent(event.key.keysym.scancode));
            inputs.menu &= ~(sdl_scancode_to_mevent(event.key.keysym.scancode));
            break;
        case SDL_QUIT:
            exit(0);
            break;
        }
    }

    // Run the game simulation.
    game_frame(&inputs);

    // Render the screen.
    softrender_context_t* context = game_draw();

    SDL_UpdateTexture(g_texture, NULL, context->buffer.data, context->buffer.width * 4);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);

    SDL_Delay(16);
}

static void clean_exit(void) {
    game_deinit();

    if (g_texture != NULL) {
        SDL_DestroyTexture(g_texture);
    }

    if (g_renderer != NULL) {
        SDL_DestroyRenderer(g_renderer);
    }

    if (g_window != NULL) {
        SDL_DestroyWindow(g_window);
    }

    SDL_Quit();
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        fprintf(stderr, "SDL2_Init failure:  %s\n", SDL_GetError());
        return 1;
    }
    atexit(clean_exit);

    g_window = SDL_CreateWindow("Portmino", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    if (g_window == NULL) {
        char buffer[8192];
        if (snprintf(buffer, sizeof(buffer), "SDL_CreateWindow failure: %s\n", SDL_GetError()) < 0) {
            abort();
        }
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Portmino", buffer, NULL);
        return 1;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, 0);
    if (g_renderer == NULL) {
        char buffer[8192];
        if (snprintf(buffer, sizeof(buffer), "SDL_Renderer failure: %s\n", SDL_GetError()) < 0) {
            abort();
        }
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Portmino", buffer, g_window);
        return 1;
    }

    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 240);
    if (g_texture == NULL) {
        char buffer[8192];
        if (snprintf(buffer, sizeof(buffer), "SDL_CreateTexture failure: %s\n", SDL_GetError()) < 0) {
            abort();
        }
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Portmino", buffer, g_window);
        return 1;
    }

    game_init();

    for (;;) {
        sdl_run();
    }

    return 1;
}
