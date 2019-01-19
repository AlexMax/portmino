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

#include "piece.h"

/**
 * Lua: Get the spawn position for a given piece config handle.
 */
static int piece_config_get_spawn_pos(lua_State* L) {
    // Parameter 1: Piece configuration handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid piece configuration handle");
    const piece_config_t* config = lua_touserdata(L, 1);
    if (config == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil piece configuration handle");
    }

    // Fetch the config and return the spawn point as a table
    lua_createtable(L, 2, 0);
    lua_pushinteger(L, config->spawn_pos.x);
    lua_rawseti(L, -2, 1);
    lua_pushinteger(L, config->spawn_pos.y);
    lua_rawseti(L, -2, 2);

    return 1;
}

/**
 * Lua: Get the spawn position for a given piece config handle.
 */
static int piece_config_get_spawn_rot(lua_State* L) {
    // Parameter 1: Piece configuration handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid piece configuration handle");
    const piece_config_t* config = lua_touserdata(L, 1);
    if (config == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil piece configuration handle");
    }

    // Fetch the config and return the spawn point as a table
    lua_pushinteger(L, config->spawn_rot);
    return 1;
}

/**
 * Push library functions into the state.
 */
int piece_openlib(lua_State* L) {
    static const luaL_Reg piecelib[] = {
        { "config_get_spawn_pos", piece_config_get_spawn_pos },
        { "config_get_spawn_rot", piece_config_get_spawn_rot },
        { NULL, NULL }
    };

    luaL_newlib(L, piecelib);

    return 1;
}
