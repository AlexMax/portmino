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

static softrender_context_t g_render_ctx;

// FIXME: Don't use a global picture
static picture_t* g_back;
static picture_t* g_field;
static picture_t* g_block;

static void softrender_init(void) {
    size_t size = RENDER_WIDTH * RENDER_HEIGHT * BITS_PER_PIXEL;

    // Initialize the buffer picture in-place.
    g_render_ctx.buffer.data = malloc(size);
    g_render_ctx.buffer.size = size;
    g_render_ctx.buffer.width = RENDER_WIDTH;
    g_render_ctx.buffer.height = RENDER_HEIGHT;

    g_back = picture_new("../res/back.png");
    g_field = picture_new("../res/field.png");
    g_block = picture_new("../res/blocks/red.png");
}

static void softrender_deinit(void) {
    if (g_back != NULL) {
        picture_delete(g_back);
        g_back = NULL;
    }

    if (g_field != NULL) {
        picture_delete(g_field);
        g_field = NULL;
    }

    if (g_block != NULL) {
        picture_delete(g_block);
        g_block = NULL;
    }

    if (g_render_ctx.buffer.data != NULL) {
        free(g_render_ctx.buffer.data);
        g_render_ctx.buffer.data = NULL;
        g_render_ctx.buffer.size = 0;
        g_render_ctx.buffer.width = 0;
        g_render_ctx.buffer.height = 0;
    }
}

static void** softrender_draw(void) {
    void* context = &g_render_ctx;

    uint8_t* buffer = g_render_ctx.buffer.data;
    memset(buffer, 0x00, g_render_ctx.buffer.size);

    picture_copy(&g_render_ctx.buffer, g_back, 0, 0);
    picture_copy(&g_render_ctx.buffer, g_field, 18, 42);
    picture_copy(&g_render_ctx.buffer, g_block, 18, 42);

    return context;
}

render_module_t soft_render_module = {
    RENDERER_SOFTWARE,
    softrender_init,
    softrender_deinit,
    softrender_draw
};
