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
#include <string.h>

#include "board.h"

/**
 * Return a random piece configuration.
 */
static piece_config_t* board_get_random_piece(board_t* board) {
    uint32_t index = random_number(&(board->next_rng), MAX_PIECES);
    return board->pieces[index];
}

/**
 * Create a new board structure.
 */
board_t *board_new(void) {
    board_t *board = malloc(sizeof(board_t));
    if (board == NULL) {
        return NULL;
    }

    // Define our configuration
    board->config.width = 10;
    board->config.height = 22;
    board->config.visible_height = 20;
    board->config.visible_nexts = 3;

    // Based on that configuration, construct the board itself
    size_t size = board->config.width * board->config.height;
    board->data.size = size;
    board->data.data = calloc(size, sizeof(uint8_t));

    // Initialize default piece configuration.
    pieces_init();

    board->pieces[0] = &g_j_piece;
    board->pieces[1] = &g_l_piece;
    board->pieces[2] = &g_s_piece;
    board->pieces[3] = &g_z_piece;
    board->pieces[4] = &g_t_piece;
    board->pieces[5] = &g_i_piece;
    board->pieces[6] = &g_o_piece;

    // Start with no piece allocated.
    board->piece = NULL;

    // Initialize the next piece PRNG.
    random_init(&board->next_rng, NULL);

    // Initialize the next piece circular buffer.
    for (size_t i = 0;i < MAX_NEXTS;i++) {
        board->nexts[i] = board_get_random_piece(board);
    }
    board->next_index = 0;

    return board;
}

/**
 * Delete a board structure.
 */
void board_delete(board_t *board) {
    if (board->piece != NULL) {
        piece_delete(board->piece);
    }

    free(board->data.data);
    free(board);
}

/**
 * Get the next piece, make it the active piece, and advance the next piece index.
 */
void board_next_piece(board_t* board) {
    if (board->piece != NULL) {
        // Free the existing piece if it exists.
        piece_delete(board->piece);
        board->piece = NULL;
    }

    // Create the next piece.
    piece_config_t* config = board->nexts[board->next_index];
    board->piece = piece_new(config);

    // Generate a new next piece in place.
    board->nexts[board->next_index] = board_get_random_piece(board);

    // Advance the next index.
    board->next_index = (board->next_index + 1) % MAX_NEXTS;
}

/**
 * Board collision test given a piece and an orientation.
 * 
 * Returns true if the piece collides with the contents of the board, otherwise
 * false.
 */
bool board_test_piece(const board_t* board, const piece_config_t* piece, int x, int y, uint8_t rot) {
    for (size_t i = 0;i < piece->data_size;i++) {
        // Get the source cell.
        uint8_t* scell = piece_get_rot(piece, rot) + i;
        if (!*scell) {
            // Source cell is empty, no collision test necessary.
            continue;
        }

        // Source cell location.
        int sx = i % piece->width;
        int sy = i / piece->width;

        // Check to see if we're off the board.
        if (x + sx < 0 || x + sx >= board->config.width || y + sy < 0 || y + sy >= board->config.height) {
            return false;
        }

        // Figure out the destination cell.
        uint8_t* dcell = board->data.data;
        dcell += y * board->config.width + x;
        dcell += sy * board->config.width + sx;

        if (!*dcell) {
            // Destination cell is empty, collision passes.
            continue;
        }

        // Got a hit!
        return false;
    }

    // All parts of the piece fit on the position on the board.
    return true;
}

/**
 * Lock a piece in a particular spot.
 * 
 * Note that no collision detection is done.  Any existing blocks will be
 * overwritten.
 */
void board_lock_piece(const board_t* board, const piece_config_t* piece, int x, int y, uint8_t rot) {
    for (size_t i = 0;i < piece->data_size;i++) {
        // Get the source cell.
        uint8_t* scell = piece_get_rot(piece, rot) + i;
        if (!*scell) {
            // Source cell is empty, do nothing.
            continue;
        }

        // Source cell location.
        int sx = i % piece->width;
        int sy = i / piece->width;

        // Check to see if we're off the board.
        if (x + sx < 0 || x + sx >= board->config.width || y + sy < 0 || y + sy >= board->config.height) {
            continue;
        }

        // Figure out the destination cell.
        uint8_t* dcell = board->data.data;
        dcell += y * board->config.width + x;
        dcell += sy * board->config.width + sx;

        // Write our piece cell into the destination cell.
        *dcell = *scell;
    }
}

/**
 * Clear the board of any lines.
 * 
 * TODO: Somehow we need to account for a fancy line-clear animation that
 *       happens over many frames.
 * 
 * Returns the number of lines cleared.
 */
uint8_t board_clear_lines(board_t* board) {
    uint8_t lines = 0;

    for (size_t i = 0;i < board->data.size;i += board->config.width) {
        // Find an empty cell in the row
        bool found_empty = false;
        for (size_t x = 0 ;x < board->config.width;x++) {
            if (!*(board->data.data + i + x)) {
                found_empty = true;
                break;
            }
        }

        if (found_empty == false) {
            // Move the contents of the board forward over the full line.
            uint8_t* dest = board->data.data + board->config.width;
            memmove(dest, board->data.data, i); // overlapping regions

            // Fill the first line with empty space.
            memset(board->data.data, 0, board->config.width);

            // We have cleared one additional line.
            lines += 1;
        }
    }

    return lines;
}
