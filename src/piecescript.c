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
#include "script.h"

/**
 * Lua: Get the position for a given piece handle.
 */
static int piecescript_get_pos(lua_State* L) {
    // Parameter 1: Piece handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid piece handle");
    const piece_t* piece = lua_touserdata(L, 1);
    if (piece == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil piece handle");
    }

    // Return the position
    script_push_vector(L, &piece->pos);
    return 1;
}

/**
 * Lua: Set the position for a given piece handle.
 */
static int piecescript_set_pos(lua_State* L) {
    // Parameter 1: Piece handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid piece handle");
    piece_t* piece = lua_touserdata(L, 1);
    if (piece == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil piece handle");
    }

    // Parameter 2: Position
    vec2i_t pos = { 0, 0 };
    bool ok = script_to_vector(L, 2, &pos);

    luaL_argcheck(L, ok, 2, "invalid position");

    piece->pos = pos;
    return 0;
}

/**
 * Lua: Get the rotation for a given piece handle.
 */
static int piecescript_get_rot(lua_State* L) {
    // Parameter 1: Piece handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid piece handle");
    const piece_t* piece = lua_touserdata(L, 1);
    if (piece == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil piece handle");
    }

    // Return the rotation as an integer
    lua_pushinteger(L, piece->rot);
    return 1;
}

/**
 * Lua: Set the position for a given piece handle.
 */
static int piecescript_set_rot(lua_State* L) {
    // Parameter 1: Piece handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid piece handle");
    piece_t* piece = lua_touserdata(L, 1);
    if (piece == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil piece handle");
    }

    // Parameter 2: Rotation
    // FIXME: Prevent out-of-bounds rotation values
    lua_Integer rot = luaL_checkinteger(L, 2);

    piece->rot = rot;
    return 0;
}

/**
 * Lua: Get the configuration userdata for a given piece handle.
 */
static int piecescript_get_config(lua_State* L) {
    // Parameter 1: Piece handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid piece handle");
    const piece_t* piece = lua_touserdata(L, 1);
    if (piece == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil piece handle");
    }

    // Return the piece config as a light userdata
    lua_pushlightuserdata(L, (void*)piece->config);
    return 1;
}

/**
 * Lua: Get the name for a given piece config handle.
 */
static int piecescript_config_get_name(lua_State* L) {
    // Parameter 1: Piece configuration handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid piece configuration handle");
    const piece_config_t* config = lua_touserdata(L, 1);
    if (config == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil piece configuration handle");
    }

    // Fetch the config and return the spawn point as a table
    lua_pushstring(L, config->name);
    return 1;
}

/**
 * Lua: Get the spawn position for a given piece config handle.
 */
static int piecescript_config_get_spawn_pos(lua_State* L) {
    // Parameter 1: Piece configuration handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid piece configuration handle");
    const piece_config_t* config = lua_touserdata(L, 1);
    if (config == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil piece configuration handle");
    }

    // Fetch the config and return the spawn point as a table
    script_push_vector(L, &config->spawn_pos);
    return 1;
}

/**
 * Lua: Get the spawn position for a given piece config handle.
 */
static int piecescript_config_get_spawn_rot(lua_State* L) {
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
 * Lua: Get the number of rotations that a piece has.
 */
static int piecescript_config_get_rot_count(lua_State* L) {
    // Parameter 1: Piece configuration handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid piece configuration handle");
    const piece_config_t* config = lua_touserdata(L, 1);
    if (config == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil piece configuration handle");
    }

    // Fetch the number of rotations and return the spawn point as a table
    lua_pushinteger(L, config->data_count);
    return 1;
}

/**
 * Push library functions into the state.
 */
int piecescript_openlib(lua_State* L) {
    static const luaL_Reg piecelib[] = {
        { "get_pos", piecescript_get_pos },
        { "set_pos", piecescript_set_pos },
        { "get_rot", piecescript_get_rot },
        { "set_rot", piecescript_set_rot },
        { "get_config", piecescript_get_config },
        { "config_get_name", piecescript_config_get_name },
        { "config_get_spawn_pos", piecescript_config_get_spawn_pos },
        { "config_get_spawn_rot", piecescript_config_get_spawn_rot },
        { "config_get_rot_count", piecescript_config_get_rot_count },
        { NULL, NULL }
    };

    luaL_newlib(L, piecelib);

    return 1;
}
