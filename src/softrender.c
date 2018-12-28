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

#include <stdlib.h>

#include "render.h"
#include "softrender.h"

/**
 * Bits per pixel of the software-rendered surface.
 */
#define MINO_SOFTRENDER_BPP 4

#define BOARD_X 18
#define BOARD_Y 42

static softrender_context_t g_render_ctx;

// FIXME: Don't use a global picture
static picture_t* g_back;
static picture_t* g_board;
static picture_t* g_blocks[8];

static void softrender_init(void) {
    size_t size = MINO_SOFTRENDER_WIDTH * MINO_SOFTRENDER_HEIGHT * MINO_SOFTRENDER_BPP;

    // Initialize the buffer picture in-place.
    g_render_ctx.buffer.data = calloc(size, sizeof(uint8_t));
    g_render_ctx.buffer.size = size;
    g_render_ctx.buffer.width = MINO_SOFTRENDER_WIDTH;
    g_render_ctx.buffer.height = MINO_SOFTRENDER_HEIGHT;

    g_back = picture_new("../res/back.png");
    g_board = picture_new("../res/board.png");
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

    if (g_board != NULL) {
        picture_delete(g_board);
        g_board = NULL;
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
    picture_copy(&g_render_ctx.buffer, g_board, BOARD_X, BOARD_Y);

    // Get our board to draw.
    board_t* board = state->boards[0];

    // The presumed size of a single empty cell in the board.  I feel
    // precalculating these sizes has a better failure mode than relying
    // on each and every block size to be correct.
    int blockx = g_board->width / board->config.width;
    int blocky = g_board->height / board->config.visible_height;

    // What index of the board do we start at?
    int start = (board->config.height - board->config.visible_height) * board->config.width;

    // Draw the board.
    for (size_t i = start;i < board->data.size;i++) {
        // What type of block are we rendering?
        uint8_t btype = board->data.data[i];
        if (!btype) {
            continue;
        }
        picture_t* bpic = g_blocks[btype];

        // What is the actual (x, y) coordinate of the block?
        int ix = i % board->config.width;
        int iy = (i / board->config.width) - (board->config.height - board->config.visible_height);

        // Draw a block.
        picture_copy(&g_render_ctx.buffer, bpic,
            BOARD_X + (blockx * ix), BOARD_Y + (blocky * iy));
    }

    // Draw the ghost piece, if any.
    if (board->ghost != NULL) {
        size_t i = board->ghost->rot * board->ghost->config->data_size;
        size_t end = i + board->ghost->config->data_size;
        for (int j = 0;i < end;i++,j++) {
            // What type of block are we rendering?
            uint8_t btype = board->ghost->config->datas[i];
            if (!btype) {
                continue;
            }
            picture_t* bpic = g_blocks[btype];

            // What is the actual (x, y) coordinate of the block?
            int ix = board->ghost->pos.x + (j % board->ghost->config->width);
            int iy = board->ghost->pos.y + (j / board->ghost->config->width);
            iy -= board->config.height - board->config.visible_height;

            if (iy < 0) {
                // Don't draw a block above the visible height.
                continue;
            }

            // Draw a block.
            picture_copy(&g_render_ctx.buffer, bpic,
                BOARD_X + (blockx * ix), BOARD_Y + (blocky * iy));
        }
    }

    // Draw the piece, if any.  The normal piece is drawn after the ghost piece
    // so it gets drawn over top of the ghost in case of overlap.
    if (board->piece != NULL) {
        size_t i = board->piece->rot * board->piece->config->data_size;
        size_t end = i + board->piece->config->data_size;
        for (int j = 0;i < end;i++,j++) {
            // What type of block are we rendering?
            uint8_t btype = board->piece->config->datas[i];
            if (!btype) {
                continue;
            }
            picture_t* bpic = g_blocks[btype];

            // What is the actual (x, y) coordinate of the block?
            int ix = board->piece->pos.x + (j % board->piece->config->width);
            int iy = board->piece->pos.y + (j / board->piece->config->width);
            iy -= board->config.height - board->config.visible_height;

            if (iy < 0) {
                // Don't draw a block above the visible height.
                continue;
            }

            // Draw a block.
            picture_copy(&g_render_ctx.buffer, bpic,
                BOARD_X + (blockx * ix), BOARD_Y + (blocky * iy));
        }
    }

    return context;
}

render_module_t soft_render_module = {
    RENDERER_SOFTWARE,
    softrender_init,
    softrender_deinit,
    softrender_draw_state
};
