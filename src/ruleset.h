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

#include "input.h"
#include "next.h"
#include "piece.h"

typedef struct board_s board_t;
typedef struct menulist_s menulist_t;
typedef struct state_s state_t;

typedef enum {
    RULESET_RESULT_OK,
    RULESET_RESULT_ERROR,
    RULESET_RESULT_TOPOUT,
} ruleset_result_t;

typedef struct ruleset_s {
    /**
     * Lua interpreter state
     * 
     * This is not an owning reference.  Do not free it.
     */
    lua_State* lua;

    /**
     * Name of the ruleset
     */
    char* name;

    /**
     * Reference to environment.
     */
    int env_ref;

    /**
     * Reference to state_frame function inside Lua.
     */
    int state_frame_ref;

    /**
     * Reference to init function inside Lua.
     */
    int init_ref;

    /**
     * Pieces loaded from Lua.
     */
    piece_configs_t* pieces;
} ruleset_t;

ruleset_t* ruleset_new(lua_State* L, const char* name);
void ruleset_delete(ruleset_t* ruleset);
menulist_t* ruleset_get_gametypes(ruleset_t* ruleset);
bool ruleset_initialize(ruleset_t* ruleset, state_t* state);
