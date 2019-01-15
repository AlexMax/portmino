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
#include "piece.h"
#include "random.h"

// Right now we only support a maximum of 8 next pieces.
#define MAX_NEXTS 8

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

    /**
     * Visible number of next pieces.
     */
    uint8_t visible_nexts;
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
     * Piece configurations.
     * 
     * These pointers are not owned by this structure.  Don't free them.
     */
    const piece_config_t* pieces[MAX_PIECES];

    /**
     * Current piece on the board.
     */
    piece_t* piece;

    /**
     * Tic that the piece spawned on the board.
     */
    uint32_t spawn_tic;

    /**
     * Ghost piece on the board.
     */
    piece_t* ghost;

    /**
     * "Next piece" PRNG.
     */
    random_t next_rng;

    /**
     * Next pieces circular buffer.
     * 
     * These pointers are not owned by this structure.  Don't free them.
     */
    const piece_config_t* nexts[MAX_NEXTS];

    /**
     * Current next piece.
     */
    uint8_t next_index;
} board_t;

board_t* board_new(void);
void board_delete(board_t* board);
const piece_config_t* board_get_next_piece(const board_t* board, size_t index);
bool board_next_piece(board_t* board, uint32_t state_tic);
bool board_test_piece(const board_t* board, const piece_config_t* piece, vec2i_t pos, uint8_t rot);
vec2i_t board_test_piece_between(const board_t* board, const piece_config_t* piece,
                                 vec2i_t src, uint8_t rot, vec2i_t dst);
void board_lock_piece(const board_t* board, const piece_config_t* piece, vec2i_t pos, uint8_t rot);
uint8_t board_clear_lines(board_t* board);
