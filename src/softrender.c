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

#define FIELD_X 18
#define FIELD_Y 42

static softrender_context_t g_render_ctx;

// FIXME: Don't use a global picture
static picture_t* g_back;
static picture_t* g_field;
static picture_t* g_blocks[8];

static void softrender_init(void) {
    size_t size = RENDER_WIDTH * RENDER_HEIGHT * BITS_PER_PIXEL;

    // Initialize the buffer picture in-place.
    g_render_ctx.buffer.data = malloc(size);
    memset(g_render_ctx.buffer.data, 0x00, g_render_ctx.buffer.size);
    g_render_ctx.buffer.size = size;
    g_render_ctx.buffer.width = RENDER_WIDTH;
    g_render_ctx.buffer.height = RENDER_HEIGHT;

    g_back = picture_new("../res/back.png");
    g_field = picture_new("../res/field.png");
    g_blocks[0] = NULL;
    g_blocks[1] = picture_new("../res/blocks/red.png");
    g_blocks[2] = picture_new("../res/blocks/orange.png");
    g_blocks[3] = picture_new("../res/blocks/yellow.png");
    g_blocks[4] = picture_new("../res/blocks/green.png");
    g_blocks[5] = picture_new("../res/blocks/cyan.png");
    g_blocks[6] = picture_new("../res/blocks/blue.png");
    g_blocks[7] = picture_new("../res/blocks/purple.png");
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

    // if (g_block != NULL) {
    //     picture_delete(g_block);
    //     g_block = NULL;
    // }

    if (g_render_ctx.buffer.data != NULL) {
        free(g_render_ctx.buffer.data);
        g_render_ctx.buffer.data = NULL;
        g_render_ctx.buffer.size = 0;
        g_render_ctx.buffer.width = 0;
        g_render_ctx.buffer.height = 0;
    }
}

static void* softrender_draw_state(const state_t* state) {
    void* context = &g_render_ctx;

    // Draw the background.
    picture_copy(&g_render_ctx.buffer, g_back, 0, 0);
    picture_copy(&g_render_ctx.buffer, g_field, FIELD_X, FIELD_Y);

    // Get our playfield to draw.
    field_t* field = state->fields[0];

    // The presumed size of a single empty cell in the playfield.  I feel
    // precalculating these sizes has a better failure mode than relying
    // on each and every block size to be correct.
    int blockx = g_field->width / field->config.width;
    int blocky = g_field->height /field->config.visible_height;

    // What index of the playfield do we start at?
    int start = (field->config.height - field->config.visible_height) * field->config.width;

    // Draw the playfield.
    for (size_t i = start;i < field->data.size;i++) {
        // What type of block are we rendering?
        uint8_t btype = field->data.data[i];
        if (!btype) {
            continue;
        }
        picture_t* bpic = g_blocks[btype];

        // What is the actual (x, y) coordinate of the block?
        int ix = i % field->config.width;
        int iy = (i / field->config.width) - (field->config.height - field->config.visible_height);

        // Draw a block.
        picture_copy(&g_render_ctx.buffer, bpic,
            FIELD_X + (blockx * ix), FIELD_Y + (blocky * iy));
    }

    return context;
}

render_module_t soft_render_module = {
    RENDERER_SOFTWARE,
    softrender_init,
    softrender_deinit,
    softrender_draw_state
};
