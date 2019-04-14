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

#include <stdlib.h>

#include "lauxlib.h"

#include "board.h"
#include "proto.h"
#include "script.h"

 /**
  * Lua: Initialize new board state.
  */
static int boardscript_new(lua_State* L) {
    // Internal State 1: registry reference
    int type = lua_getfield(L, lua_upvalueindex(1), "registry_ref");
    if (type != LUA_TNUMBER) {
        luaL_error(L, "new: missing internal state (registry_ref)");
        return 0;
    }

    // Initialize (and return) board state
    userdata_t* udata = lua_newuserdata(L, sizeof(userdata_t));
    udata->registry_ref = lua_tointeger(L, -2);
    if ((udata->data = board_new()) == NULL) {
        luaL_error(L, "Could not allocate new board.");
        return 0;
    }

    // Apply methods to the userdata
    luaL_setmetatable(L, "board_t");

    return 1;
}

/**
 * Lua: board_t finalizer.
 */
static int boardscript_delete(lua_State* L) {
    // Parameter 1: Our userdata
    userdata_t* udata = luaL_checkudata(L, 1, "board_t");

    board_delete(udata->data);
    udata->data = NULL;

    udata = NULL;
    return 0;
}

/**
 * Lua: Set a new piece for the board.
 */
static int boardscript_set_piece(lua_State* L) {
    // Parameter 1: Our userdata
    userdata_t* udata = luaL_checkudata(L, 1, "board_t");
    board_t* board = udata->data;

    // Parameter 2: Piece index
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index <= 0) {
        luaL_argerror(L, 1, "invalid piece id");
        return 0;
    }
    index -= 1;

    // Parameter 3: Piece configuration handle
    int type = lua_type(L, 3);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 3, "invalid piece configuration handle");
    const piece_config_t* config = lua_touserdata(L, 3);
    if (config == NULL) {
        luaL_argerror(L, 3, "nil piece configuration handle");
        return 0;
    }

    // Fetch the piece and return a handle to it
    piece_t* piece = board_set_piece(board, index, config);
    if (piece == NULL) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushlightuserdata(L, piece);
    return 1;
}

/**
 * Lua: Unset the piece on the board given by the index
 */
static int boardscript_unset_piece(lua_State* L) {
    // Parameter 1: Our userdata
    userdata_t* udata = luaL_checkudata(L, 1, "board_t");
    board_t* board = udata->data;

    // Parameter 2: Piece index
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index <= 0) {
        luaL_argerror(L, 1, "invalid piece id");
        return 0;
    }
    index -= 1;

    // Delete the piece.
    board_unset_piece(board, index);
    return 0;
}

/**
 * Lua: Get a board piece handle by index
 */
static int boardscript_get_piece(lua_State* L) {
    // Parameter 1: Our userdata
    userdata_t* udata = luaL_checkudata(L, 1, "board_t");
    board_t* board = udata->data;

    // Parameter 2: Piece index
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index <= 0) {
        luaL_argerror(L, 1, "invalid piece id");
        return 0;
    }
    index -= 1;

    // Fetch the piece and return a handle to it
    piece_t* piece = board_get_piece(board, index);
    if (piece == NULL) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushlightuserdata(L, piece);
    return 1;
}

/**
 * Lua: Test to see if a piece collides with a specific spot on the board.
 */
static int boardscript_test_piece(lua_State* L) {
    // Parameter 1: Our userdata
    userdata_t* udata = luaL_checkudata(L, 1, "board_t");
    board_t* board = udata->data;

    // Parameter 2: Piece name
    const char* piece_config = luaL_checkstring(L, 2);

    // Parameter 3: Position table
    vec2i_t pos = { 0, 0 };
    bool ok = script_to_vector(L, 3, &pos);
    luaL_argcheck(L, ok, 3, "invalid position");

    // Parameter 4: Rotation integer
    lua_Integer rot = luaL_checkinteger(L, 4);

    // Internal State 1: protos table
    int type = lua_rawgeti(L, LUA_REGISTRYINDEX, udata->registry_ref);
    if (type != LUA_TTABLE) {
        luaL_error(L, "missing internal state (registry_ref)");
        return 0;
    }

    type = lua_getfield(L, -1, "proto_hash");
    if (type != LUA_TTABLE) {
        luaL_error(L, "missing internal state (proto_hash)");
        return 0;
    }

    // Get the piece configuration
    lua_getfield(L, -1, piece_config);
    proto_t* proto = lua_touserdata(L, -1);
    if (proto == NULL || proto->type != MINO_PROTO_PIECE) {
        luaL_error(L, "invalid piece configuration");
        return 0;
    }

    // Actually run the test and return the result
    piece_config_t* config = proto->data;
    bool result = board_test_piece(board, config, pos, rot);
    lua_pushboolean(L, result);
    return 1;
}

/**
 * Lua: Repeatedly test collision between two points on a board, and return
 *      the last location where the piece was successfully placed.
 */
static int boardscript_test_piece_between(lua_State* L) {
    // Parameter 1: Our userdata
    userdata_t* udata = luaL_checkudata(L, 1, "board_t");
    board_t* board = udata->data;

    // Parameter 2: Piece name
    const char* piece_config = luaL_checkstring(L, 2);

    // Parameter 3: Source position
    vec2i_t src = { 0, 0 };
    bool ok = script_to_vector(L, 3, &src);
    luaL_argcheck(L, ok, 3, "invalid position");

    // Parameter 4: Rotation integer
    lua_Integer rot = luaL_checkinteger(L, 4);

    // Parameter 5: Destination position
    vec2i_t dst = { 0, 0 };
    ok = script_to_vector(L, 5, &dst);
    luaL_argcheck(L, ok, 5, "invalid position");

    // Internal State 1: protos table
    int type = lua_rawgeti(L, LUA_REGISTRYINDEX, udata->registry_ref);
    if (type != LUA_TTABLE) {
        luaL_error(L, "missing internal state (registry_ref)");
        return 0;
    }

    type = lua_getfield(L, -1, "proto_hash");
    if (type != LUA_TTABLE) {
        luaL_error(L, "missing internal state (proto_hash)");
        return 0;
    }

    // Get the piece configuration
    lua_getfield(L, -1, piece_config);
    proto_t* proto = lua_touserdata(L, -1);
    if (proto == NULL || proto->type != MINO_PROTO_PIECE) {
        luaL_error(L, "invalid piece configuration");
        return 0;
    }

    // Actually run the test and return the result
    piece_config_t* config = proto->data;
    vec2i_t result = board_test_piece_between(board, config, src, rot, dst);
    script_push_vector(L, &result);
    return 1;
}

/**
 * Lua: Test to see if a piece collides with a specific spot on the board.
 */
static int boardscript_lock_piece(lua_State* L) {
    // Parameter 1: Our userdata
    userdata_t* udata = luaL_checkudata(L, 1, "board_t");
    board_t* board = udata->data;

    // Parameter 2: Piece name
    const char* piece_config = luaL_checkstring(L, 2);

    // Parameter 3: Position table
    vec2i_t pos = { 0, 0 };
    bool ok = script_to_vector(L, 3, &pos);
    luaL_argcheck(L, ok, 3, "lock_piece: invalid position");

    // Parameter 4: Rotation integer
    lua_Integer rot = luaL_checkinteger(L, 4);

    // Internal State 1: protos table
    int type = lua_rawgeti(L, LUA_REGISTRYINDEX, udata->registry_ref);
    if (type != LUA_TTABLE) {
        luaL_error(L, "missing internal state (registry_ref)");
        return 0;
    }

    type = lua_getfield(L, -1, "proto_hash");
    if (type != LUA_TTABLE) {
        luaL_error(L, "missing internal state (proto_hash)");
        return 0;
    }

    // Get the piece configuration
    lua_getfield(L, -1, piece_config);
    proto_t* proto = lua_touserdata(L, -1);
    if (proto == NULL || proto->type != MINO_PROTO_PIECE) {
        luaL_error(L, "invalid piece configuration");
        return 0;
    }

    // Actually lock the piece
    piece_config_t* config = proto->data;
    board_lock_piece(board, config, pos, rot);
    return 0;
}

/**
 * Lua: Clear filled lines on the board.
 */
static int boardscript_clear_lines(lua_State* L) {
    // Parameter 1: Our userdata
    userdata_t* udata = luaL_checkudata(L, 1, "board_t");
    board_t* board = udata->data;

    // Clear lines and return the number of lines cleared.
    uint8_t lines = board_clear_lines(board);
    lua_pushinteger(L, lines);
    return 1;
}

int boardscript_openlib(lua_State* L) {
    static const luaL_Reg boardlib[] = {
        { "new", boardscript_new },
        { NULL, NULL }
    };

    luaL_newlib(L, boardlib);

    // Create the board_t type
    static const luaL_Reg boardtype[] = {
        { "set_piece", boardscript_set_piece },
        { "unset_piece", boardscript_unset_piece },
        { "get_piece", boardscript_get_piece },
        { "test_piece", boardscript_test_piece },
        { "test_piece_between", boardscript_test_piece_between },
        { "lock_piece", boardscript_lock_piece },
        { "clear_lines", boardscript_clear_lines },
        { NULL, NULL }
    };

    luaL_newmetatable(L, "board_t");

    luaL_newlib(L, boardtype);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, boardscript_delete);
    lua_setfield(L, -2, "__gc");

    lua_pop(L, 1);

    return 1;
}
