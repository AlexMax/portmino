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

#include "picture.h"
#include "render.h"
#include "softrender.h"

#define BITS_PER_PIXEL 4

static softrender_context_t g_render_buffer;

// FIXME: Don't use a global picture
static picture_t* g_back;
static picture_t* g_field;
static picture_t* g_block;

static void softrender_init(void) {
    size_t size = RENDER_WIDTH * RENDER_HEIGHT * BITS_PER_PIXEL;
    g_render_buffer.size = size;
    g_render_buffer.buffer = malloc(size);
    g_render_buffer.width = RENDER_WIDTH;
    g_render_buffer.height = RENDER_HEIGHT;

    g_back = picture_new("../res/back.png");
    g_field = picture_new("../res/field.png");
    g_block = picture_new("../res/blocks/red.png");
}

static void softrender_deinit(void) {
    picture_delete(g_block);
    picture_delete(g_field);
    picture_delete(g_back);

    free(g_render_buffer.buffer);
    g_render_buffer.buffer = NULL;
}

static void softbuffer_draw_picture(int x, int y, picture_t* pic) {
    int cursor = (y * g_render_buffer.width * BITS_PER_PIXEL) + (x * BITS_PER_PIXEL);
    int piccursor = 0;
    for (int i = 0;i < pic->height;i++) {
        memcpy(g_render_buffer.buffer + cursor, pic->data + piccursor, pic->width * BITS_PER_PIXEL);
        cursor += g_render_buffer.width * BITS_PER_PIXEL;
        piccursor += pic->width * BITS_PER_PIXEL;
    }
}

static void** softrender_draw(void) {
    void* context = &g_render_buffer;

    uint8_t* buffer = g_render_buffer.buffer;
    memset(buffer, 0x00, g_render_buffer.size);

    softbuffer_draw_picture(0, 0, g_back);
    softbuffer_draw_picture(18, 42, g_field);
    softbuffer_draw_picture(18, 42, g_block);

    return context;
}

render_module_t soft_render_module = {
    RENDERER_SOFTWARE,
    softrender_init,
    softrender_deinit,
    softrender_draw
};
