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

#include "define.h"

// Forward declarations.
typedef struct piece_s piece_t;
typedef struct piece_config_s piece_config_t;
typedef struct ruleset_s ruleset_t;

// Maximum number of pieces per board.
#define MAX_BOARD_PIECES 4

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

typedef struct board_s {
    /**
     * Unique id of the board.
     */
    size_t id;

    /**
     * Configuration of entire board.
     */
    board_config_t config;

    /**
     * Contents of the actual board.
     */
    board_data_t data;

    /**
     * Current pieces on the board.
     * 
     * This structure _does_ own the pieces.  Don't delete them from anywhere
     * else except inside the board structure.
     */
    piece_t* pieces[MAX_BOARD_PIECES];

    /**
     * Number of active pieces on the board.
     */
    size_t piece_count;
} board_t;

board_t* board_new(ruleset_t* ruleset, size_t board_id);
void board_delete(board_t* board);
piece_t* board_set_piece(board_t* board, size_t index, const piece_config_t* config);
void board_unset_piece(board_t* board, size_t index);
piece_t* board_get_piece(board_t* board, size_t index);
bool board_test_piece(const board_t* board, const piece_config_t* piece, vec2i_t pos, uint8_t rot);
vec2i_t board_test_piece_between(const board_t* board, const piece_config_t* piece,
                                 vec2i_t src, uint8_t rot, vec2i_t dst);
void board_lock_piece(const board_t* board, const piece_config_t* piece, vec2i_t pos, uint8_t rot);
uint8_t board_clear_lines(board_t* board);
