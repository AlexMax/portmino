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
 * Create a new board structure.
 */
board_t *board_new(ruleset_t* ruleset) {
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

    // Start with no active piece or ghost allocated.
    board->piece = NULL;
    board->ghost = NULL;

    // Initialize the next piece PRNG.
    random_init(&board->next_rng, NULL);

    // Initialize the next piece circular buffer.
    for (size_t i = 0;i < MAX_NEXTS;i++) {
        board->nexts[i] = ruleset_next_piece(ruleset);
    }
    board->next_index = 0;

    return board;
}

/**
 * Delete a board structure.
 */
void board_delete(board_t *board) {
    if (board->ghost != NULL) {
        piece_delete(board->ghost);
        board->ghost = NULL;
    }

    if (board->piece != NULL) {
        piece_delete(board->piece);
        board->piece = NULL;
    }

    free(board->data.data);
    free(board);
}

/**
 * Return the configuration of the next piece
 */
const piece_config_t* board_get_next_piece(const board_t* board, size_t index) {
    index = board->next_index + index % MAX_NEXTS;
    return board->nexts[board->next_index];
}

/**
 * "Consume" the next piece
 * 
 * Advances the next-piece index, wrapping around if necessary, and generates
 * a new next piece at the end.
 */
void board_consume_next_piece(board_t* board, ruleset_t* ruleset) {
    board->nexts[board->next_index] = ruleset_next_piece(ruleset);
    board->next_index = (board->next_index + 1) % MAX_NEXTS;
}

/**
 * Board collision test given a piece and an orientation.
 * 
 * Returns true if the piece can be placed on the board with no collision,
 * otherwise false.
 */
bool board_test_piece(const board_t* board, const piece_config_t* piece, vec2i_t pos, uint8_t rot) {
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
        if (pos.x + sx < 0 || pos.x + sx >= board->config.width ||
            pos.y + sy < 0 || pos.y + sy >= board->config.height) {
            return false;
        }

        // Figure out the destination cell.
        uint8_t* dcell = board->data.data;
        dcell += pos.y * board->config.width + pos.x;
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
 * Repeatedly test collision between two points, not including the source
 * location.  Returns the furthest point that the piece could be successfully
 * placed.
 * 
 * This function does not work with any arbitrary destination.  Only
 * destinations on the four cardinal directions from the source are accepted.
 * 
 * This function will return the source location if collision fails on the
 * first test, or if an invalid destination is supplied.
 */
vec2i_t board_test_piece_between(const board_t* board, const piece_config_t* piece,
                                 vec2i_t src, uint8_t rot, vec2i_t dst) {
    vec2i_t ret = src;

    vec2i_t delta = { 0 };
    delta.x = dst.x - src.x;
    delta.y = dst.y - src.y;

    vec2i_t pos = { 0 };
    if (delta.x != 0 && delta.y != 0) {
        // We don't do diagonals.
        return src;
    } else if (delta.x > 0) {
        // Loop along the positive x coordinate.
        for (int i = src.x + 1;i <= dst.x;i++) {
            pos.x = i;
            pos.y = ret.y;
            if (!board_test_piece(board, piece, pos, rot)) {
                return ret;
            }
            ret.x = i;
        }
    } else if (delta.x < 0) {
        // Loop along the negative x coordinate.
        for (int i = src.x - 1;i >= dst.x;i--) {
            pos.x = i;
            pos.y = ret.y;
            if (!board_test_piece(board, piece, pos, rot)) {
                return ret;
            }
            ret.x = i;
        }
    } else if (delta.y > 0) {
        // Loop along the positive y coordinate.
        for (int i = src.y + 1;i <= dst.y;i++) {
            pos.x = ret.x;
            pos.y = i;
            if (!board_test_piece(board, piece, pos, rot)) {
                return ret;
            }
            ret.y = i;
        }
    } else if (delta.y < 0) {
        // Loop along the negative y coordinate.
        for (int i = src.y - 1;i >= dst.y;i--) {
            pos.x = ret.x;
            pos.y = i;
            if (!board_test_piece(board, piece, pos, rot)) {
                return ret;
            }
            ret.y = i;
        }
    }

    // Source and destination are the same.
    return ret;
}

/**
 * Lock a piece in a particular spot.
 * 
 * Note that no collision detection is done.  Any existing blocks will be
 * overwritten.
 */
void board_lock_piece(const board_t* board, const piece_config_t* piece, vec2i_t pos, uint8_t rot) {
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
        if (pos.x + sx < 0 || pos.x + sx >= board->config.width ||
            pos.y + sy < 0 || pos.y + sy >= board->config.height) {
            continue;
        }

        // Figure out the destination cell.
        uint8_t* dcell = board->data.data;
        dcell += pos.y * board->config.width + pos.x;
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
