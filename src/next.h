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

// Forward declarations
typedef struct lua_State lua_State;
typedef struct piece_config_s piece_config_t;

// Right now we only support a maximum of 8 next pieces.
#define MAX_NEXTS 8

typedef struct next_s {
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

    /**
     * Lua state
     *
     * This is a non-owning pointer.  Do not free it.
     */
    lua_State* lua;

    /**
     * A reference to the next piece function used to populate the buffer.
     */
    int next_piece_ref;
} next_t;

bool next_init(next_t* next, lua_State* L, int next_piece_ref);
void next_deinit(next_t* next);
const piece_config_t* next_get_next_piece(const next_t* next, size_t index);
void next_consume_next_piece(next_t* next);
