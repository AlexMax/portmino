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

#include "lua.h"
#include "lauxlib.h"

#include "board.h"
#include "script.h"
#include "state.h"

/**
 * Lua: Get a board handle by board ID
 */
static int boardscript_get(lua_State* L) {
    // Parameter 1: Board number, 1-indexed
    lua_Integer board = luaL_checkinteger(L, 1);
    if (board <= 0) {
         // never returns
         luaL_argerror(L, 1, "invalid board id");
    }
    board -= 1;

    lua_pushstring(L, "state");
    if (lua_gettable(L, LUA_REGISTRYINDEX) != LUA_TLIGHTUSERDATA) {
        // never returns
        luaL_argerror(L, 1, "ruleset_get_board is missing internal state");
    }
    state_t* state = lua_touserdata(L, -1);

    if ((size_t)board >= state->board_count) {
        // never returns
        luaL_argerror(L, 1, "invalid board id");
    }

    // Return the board
    board_t* board_ptr = state->boards[board];
    lua_pushlightuserdata(L, board_ptr);
    return 1;
}

/**
 * Lua: Allocate a new piece for the board.
 */
static int boardscript_new_piece(lua_State* L) {
    // Parameter 1: Board handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid board handle");
    board_t* board = lua_touserdata(L, 1);
    if (board == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil board handle");
    }

    // Parameter 2: Piece configuration handle
    type = lua_type(L, 2);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 2, "invalid piece configuration handle");
    const piece_config_t* config = lua_touserdata(L, 2);
    if (config == NULL) {
        // never returns
        luaL_argerror(L, 2, "nil piece configuration handle");
    }

    // If a piece exists, delete it.
    if (board->piece != NULL) {
        piece_delete(board->piece);
        board->piece = NULL;
    }

    // Create a piece for the board.
    board->piece = piece_new(config);

    return 0;
}

/**
 * Lua: Delete the piece on the board given by the board handle
 */
static int boardscript_delete_piece(lua_State* L) {
    // Parameter 1: Board handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid board handle");
    board_t* board = lua_touserdata(L, 1);
    if (board == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil board handle");
    }

    // Delete the piece attached to the board
    if (board->piece != NULL) {
        piece_delete(board->piece);
        board->piece = NULL;
    }

    return 0;
}

/**
 * Lua: Get a board piece handle by board handle
 */
static int boardscript_get_piece(lua_State* L) {
    // Parameter 1: Board handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid board handle");
    board_t* board = lua_touserdata(L, 1);
    if (board == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil board handle");
    }

    // Fetch the piece and return a handle to it
    if (board->piece == NULL) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushlightuserdata(L, board->piece);
    return 1;
}

/**
 * Lua: Test to see if a piece collides with a specific spot on the board.
 */
static int boardscript_test_piece(lua_State* L) {
    // Parameter 1: Board handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid board handle");
    board_t* board = lua_touserdata(L, 1);
    if (board == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil board handle");
    }

    // Parameter 2: Piece configuration handle
    type = lua_type(L, 2);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 2, "invalid piece configuration handle");
    const piece_config_t* config = lua_touserdata(L, 2);
    if (config == NULL) {
        // never returns
        luaL_argerror(L, 2, "nil piece configuration handle");
    }

    // Parameter 3: Position table
    vec2i_t pos = { 0, 0 };
    bool ok = script_to_vector(L, 3, &pos);
    luaL_argcheck(L, ok, 3, "invalid position");

    // Parameter 4: Rotation integer
    lua_Integer rot = luaL_checkinteger(L, 4);

    // Actually run the test and return the result
    bool result = board_test_piece(board, config, pos, rot);
    lua_pushboolean(L, result);
    return 1;
}

/**
 * Lua: Repeatedly test collision between two points on a board, and return
 *      the last location where the piece was successfully placed.
 */
static int boardscript_test_piece_between(lua_State* L) {
    // Parameter 1: Board handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid board handle");
    board_t* board = lua_touserdata(L, 1);
    if (board == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil board handle");
    }

    // Parameter 2: Piece configuration handle
    type = lua_type(L, 2);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 2, "invalid piece configuration handle");
    const piece_config_t* config = lua_touserdata(L, 2);
    if (config == NULL) {
        // never returns
        luaL_argerror(L, 2, "nil piece configuration handle");
    }

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

    // Actually run the test and return the result
    vec2i_t result = board_test_piece_between(board, config, src, rot, dst);
    script_push_vector(L, &result);
    return 1;
}

/**
 * Lua: Test to see if a piece collides with a specific spot on the board.
 */
static int boardscript_lock_piece(lua_State* L) {
    // Parameter 1: Board handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid board handle");
    board_t* board = lua_touserdata(L, 1);
    if (board == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil board handle");
    }

    // Parameter 2: Piece configuration handle
    type = lua_type(L, 2);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 2, "invalid piece configuration handle");
    const piece_config_t* config = lua_touserdata(L, 2);
    if (config == NULL) {
        // never returns
        luaL_argerror(L, 2, "nil piece configuration handle");
    }

    // Parameter 3: Position table
    vec2i_t pos = { 0, 0 };
    bool ok = script_to_vector(L, 3, &pos);
    luaL_argcheck(L, ok, 3, "invalid position");

    // Parameter 4: Rotation integer
    lua_Integer rot = luaL_checkinteger(L, 4);

    // Actually lock the piece
    board_lock_piece(board, config, pos, rot);
    return 0;
}

/**
 * Lua: Test to see if a piece collides with a specific spot on the board.
 */
static int boardscript_clear_lines(lua_State* L) {
    // Parameter 1: Board handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid board handle");
    board_t* board = lua_touserdata(L, 1);
    if (board == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil board handle");
    }

    // Clear lines and return the number of lines cleared.
    uint8_t lines = board_clear_lines(board);
    lua_pushinteger(L, lines);
    return 1;
}

/**
 * Lua: Get the next piece config handle for the given board handle
 */
static int boardscript_get_next_config(lua_State* L) {
    // Parameter 1: Board handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid board handle");
    board_t* board = lua_touserdata(L, 1);
    if (board == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil board handle");
    }

    // Fetch the next piece and return a handle to its config
    const piece_config_t* config = board_get_next_piece(board, 0);
    lua_pushlightuserdata(L, (void*)config);
    return 1;
}

/**
 * Lua: Consume the next piece of the board, cycling through to the next next
 * piece and possibly generating a new next piece.
 */
static int boardscript_consume_next(lua_State* L) {
    lua_pushstring(L, "state");
    lua_gettable(L, LUA_REGISTRYINDEX);

    const state_t* state = lua_touserdata(L, -1);
    if (state == NULL) {
        // never returns
        luaL_error(L, "ruleset_get_gametic is missing internal state");
    }

    // Parameter 1: Board handle
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 1, "invalid board handle");
    board_t* board = lua_touserdata(L, 1);
    if (board == NULL) {
        // never returns
        luaL_argerror(L, 1, "nil board handle");
    }

    board_consume_next_piece(board, state->ruleset);
    return 0;
}

int boardscript_openlib(lua_State* L) {
    static const luaL_Reg boardlib[] = {
        { "get", boardscript_get },
        { "new_piece", boardscript_new_piece },
        { "delete_piece", boardscript_delete_piece },
        { "get_piece", boardscript_get_piece },
        { "test_piece", boardscript_test_piece },
        { "test_piece_between", boardscript_test_piece_between },
        { "lock_piece", boardscript_lock_piece },
        { "clear_lines", boardscript_clear_lines },
        { "get_next_config", boardscript_get_next_config },
        { "consume_next", boardscript_consume_next },
        { NULL, NULL }
    };

    luaL_newlib(L, boardlib);
    return 1;
}
