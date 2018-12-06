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

#include "game.h"
#include "render.h"
#include "softrender.h"

static SDL_Window* g_window;
static SDL_Renderer* g_renderer;
static SDL_Texture* g_texture;

static render_module_t* g_render_module;

static void clean_exit(void) {
    if (g_render_module != NULL) {
        render_deinit(g_render_module);
    }

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

static void sdl_run(void) {
    softrender_context_t* context;
    g_render_module->draw(&context);

    SDL_UpdateTexture(g_texture, NULL, context->buffer, context->width * 4);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);

    SDL_Delay(5000);
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

    g_render_module = render_init();

    //for (;;) {
        sdl_run();
    //}

    return 1;
}
