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

#include "state.h"

#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"

#include "board.h"
#include "error.h"
#include "gametype.h"
#include "ruleset.h"

/**
 * Create our gamestate
 */
state_t* state_new(ruleset_t* ruleset, gametype_t* gametype) {
    state_t* state = NULL;

    // Allocate the actual state.
    if ((state = calloc(1, sizeof(state_t))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    state->tic = 0;
    state->state_table_ref = LUA_REFNIL;
    state->lua = ruleset->lua;
    state->background = NULL;
    state->player_count = 1;
    state->ruleset = ruleset;
    state->gametype = gametype;

    // Create a table to hold our state, and create a reference to it.
    lua_createtable(state->lua, 0, 0);
    if ((state->state_table_ref = luaL_ref(state->lua, LUA_REGISTRYINDEX)) == LUA_REFNIL) {
        error_push_allocerr();
        goto fail;
    }

    return state;

fail:
    state_delete(state);
    return NULL;
}

/**
 * Delete our gamestate
 */
void state_delete(state_t* state) {
    if (state == NULL) {
        return;
    }

    luaL_unref(state->lua, LUA_REGISTRYINDEX, state->state_table_ref);

    free(state->background);
    state->background = NULL;

    free(state);
}

/**
 * Stuff to run when we're initializing a new game
 */
bool state_initgame(state_t* state) {
    if (ruleset_initialize(state->ruleset, state) == false) {
        return false;
    }
    if (gametype_initialize(state->gametype, state) == false) {
        return false;
    }

    return true;
}

/**
 * Stuff to run on every frame that's outside the scope of Lua
 */
bool state_frame(state_t* state) {
    // Whatever happens, our gametic increases by one.  Tic 0 does not exist.
    state->tic += 1;

    if (state->tic == 0) {
        // How have you been playing for this long?
        return false;
    }

    return true;
}
