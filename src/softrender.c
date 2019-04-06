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

#include "softrender.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "error.h"
#include "piece.h"
#include "render.h"
#include "softblock.h"
#include "softfont.h"
#include "vfs.h"

// Forward declarations
static void softrender_deinit(void);

static softrender_context_t g_render_ctx;

// FIXME: Don't use a global picture
static picture_t* g_back;
static picture_t* g_board;
static softblock_t* g_block;
static softfont_t* g_font;

static bool softrender_init(void) {
    size_t size = MINO_SOFTRENDER_WIDTH * MINO_SOFTRENDER_HEIGHT * MINO_SOFTRENDER_BPP;

    // Initialize the buffer picture in-place.
    if ((g_render_ctx.buffer.data = calloc(size, sizeof(uint8_t))) == NULL) {
        goto fail;
    }
    g_render_ctx.buffer.size = size;
    g_render_ctx.buffer.width = MINO_SOFTRENDER_WIDTH;
    g_render_ctx.buffer.height = MINO_SOFTRENDER_HEIGHT;

    if ((g_back = picture_new_vfs("background/default/1.png")) == NULL) {
        goto fail;
    }

    if ((g_board = picture_new_vfs("interface/default/board.png")) == NULL) {
        goto fail;
    }

    // Board picture has adjustable transparency.
    for (size_t i = 0;i < g_board->size;i += MINO_SOFTRENDER_BPP) {
        g_board->data[i + 3] = 192;
    }

    if ((g_block = softblock_new("block/default/8px.png")) == NULL) {
        goto fail;
    }

    if ((g_font = softfont_new("interface/default/font.png")) == NULL) {
        goto fail;
    }

    return true;

fail:
    softrender_deinit();
    error_push("Could not initialize software renderer.");
    return false;
}

static void softrender_deinit(void) {
    picture_delete(g_back);
    g_back = NULL;

    picture_delete(g_board);
    g_board = NULL;

    softblock_delete(g_block);
    g_block = NULL;

    softfont_delete(g_font);
    g_font = NULL;

    free(g_render_ctx.buffer.data);
    g_render_ctx.buffer.data = NULL;
    g_render_ctx.buffer.size = 0;
    g_render_ctx.buffer.width = 0;
    g_render_ctx.buffer.height = 0;
}

/**
 * Return the rendering context
 */
static void* softrender_context(void) {
    return &g_render_ctx;
}

/**
 * Clear the buffer
 */
static void softrender_clear(void) {
    memset(g_render_ctx.buffer.data, 0x00, g_render_ctx.buffer.size);
}

/**
 * Draw the background picture
 */
static void softrender_draw_background(void) {
    // Draw the background.
    picture_copy(&g_render_ctx.buffer, vec2i_zero(), g_back, vec2i_zero());
}

/**
 * Draw a board and any attached pieces on the screen using the software renderer
 */
static void softrender_draw_board(vec2i_t pos, const board_t* board) {
    picture_blit(&g_render_ctx.buffer, vec2i(pos.x, pos.y), g_board, vec2i_zero());

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
            vec2i(pos.x + (blockx * ix), pos.y + (blocky * iy)),
            bpic, vec2i_zero());
    }

    // Draw pieces, if any.  The normal piece is drawn after the ghost piece
    // so it gets drawn over top of the ghost in case of overlap.
    for (size_t i = 0;i < MAX_BOARD_PIECES;i++) {
        if (board->pieces[i] != NULL) {
            const piece_t* piece = board->pieces[i];

            size_t i = piece->rot * piece->config->data_size;
            size_t end = i + piece->config->data_size;
            for (int j = 0;i < end;i++, j++) {
                // What type of block are we rendering?
                uint8_t btype = piece->config->data[i];
                if (!btype) {
                    continue;
                }
                picture_t* bpic = softblock_get(g_block, --btype);

                // What is the actual (x, y) coordinate of the block?
                int ix = piece->pos.x + (j % piece->config->width);
                int iy = piece->pos.y + (j / piece->config->width);
                iy -= board->config.height - board->config.visible_height;

                if (iy < 0) {
                    // Don't draw a block above the visible height.
                    continue;
                }

                // Draw a block.
                picture_blit(&g_render_ctx.buffer,
                    vec2i(pos.x + (blockx * ix), pos.y + (blocky * iy)),
                    bpic, vec2i_zero());
            }
        }
    }
}

/**
 * Draw text using the software renderer
 */
static void softrender_draw_font(vec2i_t pos, const char* text) {
    softfont_render(g_font, &g_render_ctx.buffer, pos, text);
}

/**
 * Draw a cool background image for the main menu
 */
static void softrender_draw_mainmenu_bg(void) {
    static uint8_t mainmenu[MINO_SOFTRENDER_WIDTH * MINO_SOFTRENDER_HEIGHT];
    static uint8_t mmpalette[4 * 3];
    static bool initialized;
    static uint32_t frame;
    static uint8_t cycle[120]; // Two seconds worth

    if (!initialized) {
        // Generate random noise for our background.
        for (size_t i = 0;i < sizeof(mainmenu);i++) {
            mainmenu[i] = rand() % 4;
        }

        // Generate palette cycle
        int a = 192 - 108;
        double s = (double)a / sizeof(cycle);
        for (size_t i = 0;i < sizeof(cycle);i++) {
            double is = i * s * 2;
            cycle[i] = (a - abs((int)is % (2 * a) - a)) + 108;
        }

        initialized = true;
    } else {
        // Cycle the palette
        mmpalette[0] = cycle[frame % sizeof(cycle)];
        mmpalette[3] = cycle[(frame + 30) % sizeof(cycle)];
        mmpalette[6] = cycle[(frame + 60) % sizeof(cycle)];
        mmpalette[9] = cycle[(frame + 90) % sizeof(cycle)];
    }

    // Draw using our current colors.
    for (size_t i = 0, j = 0;i < sizeof(mainmenu);i++, j += MINO_SOFTRENDER_BPP) {
        size_t color = mainmenu[i];
        size_t colorindex = color * 3;
        g_render_ctx.buffer.data[j] = mmpalette[colorindex];
        g_render_ctx.buffer.data[j + 1] = mmpalette[colorindex + 1];
        g_render_ctx.buffer.data[j + 2] = mmpalette[colorindex + 2];
        g_render_ctx.buffer.data[j + 3] = 0xFF;
    }

    // Increase our frame by one.
    frame++;
}

/**
 * Draw a piece at an arbitrary location
 */
static void softrender_draw_piece(vec2i_t pos, const piece_config_t* piece) {
    // We have no board, so...hardcode this?
    int blockx = 8;
    int blocky = 8;

    size_t i = piece->spawn_rot * piece->data_size;
    size_t end = i + piece->data_size;
    for (int j = 0;i < end;i++, j++) {
        // What type of block are we rendering?
        uint8_t btype = piece->data[i];
        if (!btype) {
            continue;
        }
        picture_t* bpic = softblock_get(g_block, --btype);

        // What is the actual (x, y) coordinate of the block?
        int ix = piece->spawn_pos.x + (j % piece->width);
        int iy = 0 + (j / piece->width);

        // Draw a block of the next piece.
        picture_blit(&g_render_ctx.buffer,
            vec2i(pos.x + (blockx * ix), pos.y + (blocky * iy)),
            bpic, vec2i_zero());
    }
}

render_module_t softrender_module = {
    RENDERER_SOFTWARE,
    softrender_init,
    softrender_deinit,
    softrender_context,
    softrender_clear,
    softrender_draw_background,
    softrender_draw_board,
    softrender_draw_font,
    softrender_draw_mainmenu_bg,
    softrender_draw_piece,
};
