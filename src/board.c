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

#include "board.h"

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

    // Based on that configuration, construct the board itself
    size_t size = board->config.width * board->config.height;
    board->data.size = size;
    board->data.data = calloc(size, sizeof(uint8_t));

    // Start with no piece allocated.
    board->piece = NULL;
    board->piece = piece_new_s();

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
