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

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "piece.h"

/**
 * Configuration variables for the board.
 */
typedef struct {
    /**
     * Width of the board.
     */
    int16_t width;

    /**
     * Complete height of the board.
     */
    int16_t height;

    /**
     * Visible height of the board, starting from the bottom.
     */
    int16_t visible_height;
} board_config_t;

typedef struct {
    /**
     * Size of the data member.  Usually the width * height of the board.
     */
    size_t size;

    /**
     * Board data of 'size' length.
     */
    uint8_t* data;
} board_data_t;

typedef struct {
    /**
     * Configuration of entire board.
     */
    board_config_t config;

    /**
     * Contents of the actual board.
     */
    board_data_t data;

    /**
     * Current piece on the board.
     */
    piece_t* piece;
} board_t;

board_t* board_new(void);
void board_delete(board_t* board);
bool board_test_piece(const board_t* board, const piece_config_t* piece, int x, int y, uint8_t rot);
void board_lock_piece(const board_t* board, const piece_config_t* piece, int x, int y, uint8_t rot);
