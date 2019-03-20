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
typedef struct lua_State lua_State;
typedef struct piece_configs_s piece_configs_t;

typedef struct environment_s {
    /**
     * Pointer to lua state.
     *
     * This pointer is not owned by this structure.  Do not free it.
     */
    lua_State* lua;

    /**
     * Reference to environment.
     */
    int env_ref;

    /**
     * Reference to ruleset module.
     */
    int ruleset_ref;

    /**
     * Pieces loaded from Lua.
     */
    piece_configs_t* pieces;
} environment_t;

environment_t* environment_new(lua_State* L, const char* ruleset, const char* gametype);
void environment_delete(environment_t* env);
bool environment_start(environment_t* env);
