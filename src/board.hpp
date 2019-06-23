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

#include "define.hpp"

// Forward declarations.
typedef struct entity_s entity_t;
typedef struct lua_State lua_State;
typedef struct piece_s piece_t;
typedef struct piece_config_s piece_config_t;
typedef struct ruleset_s ruleset_t;

// Maximum number of pieces per board.
#define MAX_BOARD_PIECES 4

typedef struct {
    /**
     * A reference to the piece entity
     */
    int piece_ref;

    /**
     * The cached piece entity
     */
    piece_t* piece;

    /**
     * Current position of the piece, origin is at the top-left.  Can be off
     * the side of the board.
     */
    vec2i_t pos;

    /**
     * Current orientation of the piece.
     */
    uint8_t rot;

    /**
     * True if piece should be rendered as a ghost piece, otherwise false.
     */
    uint8_t alpha;
} boardpiece_t;

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
     * Lua state.
     *
     * Necessary for deleting piece references.
     */
    lua_State* lua;

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
    boardpiece_t pieces[MAX_BOARD_PIECES];

    /**
     * Number of active pieces on the board.
     */
    size_t piece_count;
} board_t;

board_t* board_new(lua_State* L);
void board_delete(board_t* board);
uint8_t board_get(board_t* board, vec2i_t pos);
int board_get_piece_ref(board_t* board, size_t index);
bool board_set_piece(board_t* board, size_t index, piece_t* piece, int piece_ref);
bool board_unset_piece(board_t* board, size_t index);
boardpiece_t* board_get_boardpiece(board_t* board, size_t index);
bool board_test_piece(const board_t* board, const piece_config_t* piece, vec2i_t pos, uint8_t rot);
vec2i_t board_test_piece_between(const board_t* board, const piece_config_t* piece,
                                 vec2i_t src, uint8_t rot, vec2i_t dst);
void board_lock_piece(const board_t* board, const piece_config_t* piece, vec2i_t pos, uint8_t rot);
uint8_t board_clear_lines(board_t* board);
buffer_t* board_serialize(board_t* board);
void board_entity_init(entity_t* entity);
