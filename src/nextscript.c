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

#include "lauxlib.h"

#include "next.h"
#include "state.h"

static int nextscript_get(lua_State* L) {
    // Parameter 1: Board number, 1-indexed
    lua_Integer next = luaL_checkinteger(L, 1);
    if (next <= 0) {
         // never returns
         luaL_argerror(L, 1, "invalid board id");
    }
    next -= 1;

    lua_pushstring(L, "state");
    if (lua_gettable(L, LUA_REGISTRYINDEX) != LUA_TLIGHTUSERDATA) {
        // never returns
        luaL_argerror(L, 1, "nextscript_get is missing internal state");
    }
    state_t* state = lua_touserdata(L, -1);

    if ((size_t)next >= state->next_count) {
        // never returns
        luaL_argerror(L, 1, "invalid next id");
    }

    // Return the board
    next_t* next_ptr = state->nexts[next];
    lua_pushlightuserdata(L, next_ptr);
    return 1;
}

/**
 * Lua: Get the next piece config handle for the given next handle
 */
static int nextscript_get_next_config(lua_State* L) {
    // Parameter 1: Next handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid next handle");
    next_t* next = lua_touserdata(L, 1);
    if (next == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil next handle");
    }

    // Fetch the next piece and return a handle to its config
    const piece_config_t* config = next_get_next_piece(next, 0);
    lua_pushlightuserdata(L, (void*)config);
    return 1;
}

/**
 * Lua: Consume the next piece of the next struct, cycling through to the
 * _next_ next piece and generating a new next piece.
 */
static int nextscript_consume_next(lua_State* L) {
    // Parameter 1: Next handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid next handle");
    next_t* next = lua_touserdata(L, 1);
    if (next == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil next handle");
    }

    lua_pushstring(L, "state");
    if (lua_gettable(L, LUA_REGISTRYINDEX) != LUA_TLIGHTUSERDATA) {
        // never returns
        luaL_argerror(L, 1, "nextscript_consume_next is missing internal state");
    }
    state_t* state = lua_touserdata(L, -1);

    next_consume_next_piece(next, state->ruleset);
    return 0;
}

int next_openlib(lua_State* L) {
    static const luaL_Reg nextlib[] = {
        { "get", nextscript_get },
        { "get_next_config", nextscript_get_next_config },
        { "consume_next", nextscript_consume_next },
        { NULL, NULL }
    };

    luaL_newlib(L, nextlib);
    return 1;
}
