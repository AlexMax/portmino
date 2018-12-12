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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"
#include "softrender.h"

#define BITS_PER_PIXEL 4

static softrender_context_t g_render_buffer;

static void softrender_init(void) {
    size_t size = RENDER_WIDTH * RENDER_HEIGHT * BITS_PER_PIXEL;
    g_render_buffer.size = size;
    g_render_buffer.buffer = malloc(size);
    g_render_buffer.width = RENDER_WIDTH;
    g_render_buffer.height = RENDER_HEIGHT;
}

static void softrender_deinit(void) {
    free(g_render_buffer.buffer);
    g_render_buffer.buffer = NULL;
}

static void** softrender_draw(void) {
    void* context = &g_render_buffer;

    uint8_t* buffer = g_render_buffer.buffer;
    memset(buffer, 0x88, g_render_buffer.size);

    for (int i = 0, j = 0;i < g_render_buffer.size;i += 4, j += 1) {
        buffer[i] = j % 256;
        buffer[i + 1] = j % 256;
        buffer[i + 2] = j % 256;
        buffer[i + 3] = 255;
    }

    return context;
}

render_module_t soft_render_module = {
    RENDERER_SOFTWARE,
    softrender_init,
    softrender_deinit,
    softrender_draw
};
