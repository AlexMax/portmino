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

#include "lua.h"

#include "define.h"

// Right now we only support seven piece configurations.
#define MAX_PIECES 7

#define ROT_0 0
#define ROT_R 1
#define ROT_2 2
#define ROT_L 3

typedef struct {
    /**
     * Name of the piece.
     */
    char* name;

    /**
     * What a piece looks like from every rotation.
     * 
     * Total size of this pointer is data_size * data_count.
     */
    uint8_t* datas;

    /**
     * Total width of a single piece, including blank space.
     */
    uint8_t width;

    /**
     * Total height of a single piece, including blank space.
     */
    uint8_t height;

    /**
     * The individual size of each data entry in bytes.
     */
    size_t data_size;

    /**
     * How many rotations a piece has.  Usually 4.
     */
    uint8_t data_count;

    /**
     * Spawn position of piece.
     */
    vec2i_t spawn_pos;

    /**
     * Initial rotation of piece.
     */
    uint8_t spawn_rot;
} piece_config_t;

typedef struct {
    piece_config_t** configs;
    size_t size;
} piece_configs_t;

typedef struct {
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
     * Configuration of the piece.
     * 
     * The piece does not own this pointer, so don't malloc or free it.
     */
    const piece_config_t* config;
} piece_t;

piece_configs_t* piece_configs_new(lua_State* L);
void piece_configs_delete(piece_configs_t* pieces);
void pieces_init(void);
void pieces_deinit(void);
piece_t* piece_new(const piece_config_t* config);
void piece_delete(piece_t* piece);
uint8_t* piece_get_rot(const piece_config_t* piece, uint8_t rot);
