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
#include "softblock.h"
#include "softfont.h"
#include "softrender.h"
#include "vfs.h"

#define BOARD_X 18
#define BOARD_Y 42

#define NEXT_X_START 18
#define NEXT_Y (BOARD_Y - 20)

static softrender_context_t g_render_ctx;

// FIXME: Don't use a global picture
static picture_t* g_back;
static picture_t* g_board;
static softblock_t* g_block;
static softfont_t* g_font;

#include <string.h>

static void softrender_init(void) {
    size_t size = MINO_SOFTRENDER_WIDTH * MINO_SOFTRENDER_HEIGHT * MINO_SOFTRENDER_BPP;

    // Initialize the buffer picture in-place.
    g_render_ctx.buffer.data = calloc(size, sizeof(uint8_t));
    g_render_ctx.buffer.size = size;
    g_render_ctx.buffer.width = MINO_SOFTRENDER_WIDTH;
    g_render_ctx.buffer.height = MINO_SOFTRENDER_HEIGHT;

    g_back = picture_new_vfs("background/default/1.png");

    g_board = picture_new_vfs("interface/default/board.png");
    // Board picture has adjustable transparency.
    for (size_t i = 0;i < g_board->size;i += MINO_SOFTRENDER_BPP) {
        g_board->data[i + 3] = 192;
    }

    g_block = softblock_new("block/default.png");

    g_font = softfont_new("interface/default/font.png");
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

    if (g_block != NULL) {
        softblock_delete(g_block);
        g_block = NULL;
    }

    if (g_font != NULL) {
        softfont_delete(g_font);
        g_font = NULL;
    }

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
    picture_copy(&g_render_ctx.buffer, vec2i_zero(), g_back, vec2i_zero());
    picture_blit(&g_render_ctx.buffer, vec2i(BOARD_X, BOARD_Y), g_board, vec2i_zero());

    // Get our board to draw.
    board_t* board = state->boards[0];

    // The presumed size of a single empty cell in the board.  I feel
    // precalculating these sizes has a better failure mode than relying
    // on each and every block size to be correct.
    int blockx = g_board->width / board->config.width;
    int blocky = g_board->height / board->config.visible_height;

    // What index of the board do we start at?
    int start = (board->config.height - board->config.visible_height) * board->config.width;

    // Draw the blocks on the board.
    for (size_t i = start;i < board->data.size;i++) {
        // What type of block are we rendering?
        uint8_t btype = board->data.data[i];
        if (!btype) {
            continue;
        }
        picture_t* bpic = softblock_get(g_block, --btype);

        // What is the actual (x, y) coordinate of the block?
        int ix = i % board->config.width;
        int iy = (i / board->config.width) - (board->config.height - board->config.visible_height);

        // Draw a block.
        picture_blit(&g_render_ctx.buffer,
            vec2i(BOARD_X + (blockx * ix), BOARD_Y + (blocky * iy)),
            bpic, vec2i_zero());
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
            picture_t* bpic = softblock_get(g_block, --btype);

            // What is the actual (x, y) coordinate of the block?
            int ix = board->ghost->pos.x + (j % board->ghost->config->width);
            int iy = board->ghost->pos.y + (j / board->ghost->config->width);
            iy -= board->config.height - board->config.visible_height;

            if (iy < 0) {
                // Don't draw a block above the visible height.
                continue;
            }

            // Draw a block.
            picture_blit(&g_render_ctx.buffer,
                vec2i(BOARD_X + (blockx * ix), BOARD_Y + (blocky * iy)),
                bpic, vec2i_zero());
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
            picture_t* bpic = softblock_get(g_block, --btype);

            // What is the actual (x, y) coordinate of the block?
            int ix = board->piece->pos.x + (j % board->piece->config->width);
            int iy = board->piece->pos.y + (j / board->piece->config->width);
            iy -= board->config.height - board->config.visible_height;

            if (iy < 0) {
                // Don't draw a block above the visible height.
                continue;
            }

            // Draw a block.
            picture_blit(&g_render_ctx.buffer,
                vec2i(BOARD_X + (blockx * ix), BOARD_Y + (blocky * iy)),
                bpic, vec2i_zero());
        }
    }

    // Draw the next piece.
    piece_config_t* next = board_get_next_piece(board, 0);
    if (next != NULL) {
        size_t i = next->spawn_rot * next->data_size;
        size_t end = i + next->data_size;
        for (int j = 0;i < end;i++,j++) {
            // What type of block are we rendering?
            uint8_t btype = next->datas[i];
            if (!btype) {
                continue;
            }
            picture_t* bpic = softblock_get(g_block, --btype);

            // What is the actual (x, y) coordinate of the block?
            int ix = next->spawn_pos.x + (j % next->width);
            int iy = 0 + (j / next->width);

            // Draw a block of the next piece.
            picture_blit(&g_render_ctx.buffer,
                vec2i(NEXT_X_START + (blockx * ix), NEXT_Y + (blocky * iy)),
                bpic, vec2i_zero());
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
