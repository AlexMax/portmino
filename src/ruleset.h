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

#include "event.h"
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
     * Reference to state function inside Lua.
     */
    int state_ref;

    /**
     * Pieces loaded from Lua.
     */
    piece_configs_t* pieces;

    /**
     * Reference to next piece function inside Lua.
     */
    int next_piece_ref;
} ruleset_t;

ruleset_t* ruleset_new(lua_State* L, const char* name);
void ruleset_delete(ruleset_t* ruleset);
menulist_t* ruleset_get_gametypes(ruleset_t* ruleset);
ruleset_result_t ruleset_frame(ruleset_t* ruleset, state_t* state,
                               const playerevents_t* playerevents);
const piece_config_t* ruleset_next_piece(ruleset_t* ruleset, next_t* next);
