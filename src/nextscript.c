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

/**
 * Lua: Initialize new next state.
 */
static int nextscript_new(lua_State* L) {
    // Parameter 1: Our "next piece" function
    luaL_checktype(L, 1, LUA_TFUNCTION);

    // Create a ref from our function
    int next_ref = luaL_ref(L, 1);
    if (next_ref == LUA_NOREF) {
        luaL_error(L, "Could not allocate reference to next piece function.");
        return 0;
    }

    // Initialize (and return) next state
    next_t* next = lua_newuserdata(L, sizeof(next_t));
    if (next_init(next, L, next_ref) == false) {
        luaL_error(L, "Could not allocate new next.");
        return 0;
    }

    // Apply methods to the userdata
    luaL_setmetatable(L, "next_t");

    return 1;
}

/**
 * Lua: Get the next piece config handle for the given next handle
 */
static int nextscript_get_next_config(lua_State* L) {
    // Parameter 1: Our userdata
    next_t* next = luaL_checkudata(L, 1, "next_t");

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
    // Parameter 1: Our userdata
    next_t* next = luaL_checkudata(L, 1, "next_t");

    next_consume_next_piece(next);
    return 0;
}

int nextscript_openlib(lua_State* L) {
    static const luaL_Reg nextlib[] = {
        { "new", nextscript_new },
        { NULL, NULL }
    };

    luaL_newlib(L, nextlib);

    // Create the next_t type
    static const luaL_Reg nexttype[] = {
        { "get_next_config", nextscript_get_next_config },
        { "consume_next", nextscript_consume_next },
        { NULL, NULL }
    };

    luaL_newmetatable(L, "next_t");
    luaL_newlib(L, nexttype);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    return 1;
}
