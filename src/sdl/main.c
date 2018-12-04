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

#include <SDL.h>

#include "game.h"

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        fprintf(stderr, "SDL2_Init failure:  %s\n", SDL_GetError());
        return 1;
    }
    atexit(SDL_Quit);

    SDL_Window* window = SDL_CreateWindow("Portmino", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, 0);
    if (window == NULL) {
        char buffer[8192];
        if (snprintf(buffer, sizeof(buffer), "SDL_CreateWindow failure: %s\n", SDL_GetError()) < 0) {
            abort();
        }
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Portmino", buffer, NULL);
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        char buffer[8192];
        if (snprintf(buffer, sizeof(buffer), "SDL_Renderer failure: %s\n", SDL_GetError()) < 0) {
            abort();
        }
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Portmino", buffer, window);
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 320, 240);
    if (texture == NULL) {
        char buffer[8192];
        if (snprintf(buffer, sizeof(buffer), "SDL_CreateTexture failure: %s\n", SDL_GetError()) < 0) {
            abort();
        }
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Portmino", buffer, window);
        return 1;
    }

    static uint8_t buffer[320 * 240 * 4];
    memset(&buffer, 0, sizeof(buffer));

    for (int i = 0, j = 0;i < sizeof(buffer);i += 4, j += 1) {
        buffer[i] = j % 256;
        buffer[i + 1] = j % 256;
        buffer[i + 2] = j % 256;
        buffer[i + 3] = SDL_ALPHA_OPAQUE;
    }

    SDL_UpdateTexture(texture, NULL, &buffer[0], 320 * 4);

    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Delay(5000);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
