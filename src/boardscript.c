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
#include "entity.h"
#include "entityscript.h"
#include "proto.h"
#include "script.h"

 /**
  * Lua: Initialize new board state.
  */
static int boardscript_create(lua_State* L) {
    // Internal State 1: Entity manager
    int type = lua_getfield(L, lua_upvalueindex(1), "entity_manager");
    if (type != LUA_TLIGHTUSERDATA) {
        luaL_error(L, "missing internal state (entity_manager)");
        return 0;
    }
    entity_manager_t* manager = lua_touserdata(L, -1);

    // Allocate the entity
    entity_t* entity = entity_manager_create(manager);
    if (entity == NULL) {
        luaL_error(L, "could not allocate new entity");
        return 0;
    }

    // Initialize the entity with random state
    bool ok = board_entity_init(entity, manager);
    if (ok == false) {
        luaL_error(L, "could not initialize entity");
        return 0;
    }

    // Allocate a handle for our entity
    handle_t* id = lua_newuserdata(L, sizeof(handle_t));
    *id = entity->id;

    // Designate as an entity handle
    luaL_setmetatable(L, "handle_t");
    return 1;
}

/**
 * Lua: Get the value of a specific x, y position on the board
 */
static int boardscript_get(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Parameter 2: Position to check
    vec2i_t pos = vec2i_zero();
    if (script_to_vector(L, 2, &pos) == false) {
        luaL_argerror(L, 2, "position is not a valid vector");
        return 0;
    }

    // Return board data
    lua_pushinteger(L, board_get(board, pos));
    return 1;
}

/**
 * Lua: Get a board piece handle by index
 */
static int boardscript_get_piece(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Parameter 2: Piece index
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index <= 0) {
        luaL_argerror(L, 2, "invalid piece id");
        return 0;
    }
    index -= 1;

    // Fetch the piece and return a handle to it
    int piece_ref = board_get_piece_ref(board, index);
    if (piece_ref == LUA_NOREF) {
        lua_pushnil(L);
        return 1;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, piece_ref);
    return 1;
}

/**
 * Lua: Set a piece into a position on the board.
 */
static int boardscript_set_piece(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Parameter 2: Piece index
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index <= 0) {
        luaL_argerror(L, 2, "invalid piece id");
        return 0;
    }
    index -= 1;

    // Parameter 3: Piece userdata
    entity_t* pentity = entityscript_to_entity(L, 3, MINO_ENTITY_PIECE);
    piece_t* piece = pentity->data;

    int piece_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    board_set_piece(board, index, pentity->id);

    return 0;
}

/**
 * Lua: Unset the piece on the board given by the index
 */
static int boardscript_unset_piece(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Parameter 2: Piece index
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index <= 0) {
        luaL_argerror(L, 2, "invalid piece id");
        return 0;
    }
    index -= 1;

    // Delete the piece.
    board_unset_piece(board, index);
    return 0;
}

/**
 * Lua: Get piece position
 */
static int boardscript_get_pos(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Parameter 2: Piece index
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index <= 0) {
        luaL_argerror(L, 2, "invalid piece id");
        return 0;
    }
    index -= 1;

    // Get the piece position
    boardpiece_t* piece = board_get_boardpiece(board, index);
    if (piece == NULL) {
        luaL_error(L, "no piece in this board index");
        return 0;
    }
    script_push_vector(L, &piece->pos);
    return 1;
}

/**
 * Lua: Set piece position
 */
static int boardscript_set_pos(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Parameter 2: Piece index
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index <= 0) {
        luaL_argerror(L, 2, "invalid piece id");
        return 0;
    }
    index -= 1;

    // Parameter 3: Position
    luaL_checktype(L, 3, LUA_TTABLE);

    // Set the piece position
    boardpiece_t* piece = board_get_boardpiece(board, index);
    if (piece == NULL) {
        luaL_error(L, "no piece in this board index");
        return 0;
    }
    vec2i_t pos = vec2i_zero();
    if (script_to_vector(L, 3, &pos) == false) {
        luaL_argerror(L, 3, "position is not a valid vector");
        return 0;
    }
    piece->pos = pos;
    return 0;
}

/**
 * Lua: Get piece rotation
 */
static int boardscript_get_rot(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Parameter 2: Piece index
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index <= 0) {
        luaL_argerror(L, 2, "invalid piece id");
        return 0;
    }
    index -= 1;

    // Get the piece rotation
    boardpiece_t* piece = board_get_boardpiece(board, index);
    if (piece == NULL) {
        luaL_error(L, "no piece in this board index");
        return 0;
    }
    lua_pushinteger(L, piece->rot);
    return 1;
}

/**
 * Lua: Set piece rotation
 */
static int boardscript_set_rot(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Parameter 2: Piece index
    lua_Integer index = luaL_checkinteger(L, 2);
    if (index <= 0) {
        luaL_argerror(L, 2, "invalid piece id");
        return 0;
    }
    index -= 1;

    // Parameter 3: Piece rotation
    lua_Integer rot = luaL_checkinteger(L, 3);

    // Set the piece rotation
    boardpiece_t* piece = board_get_boardpiece(board, index);
    if (piece == NULL) {
        luaL_error(L, "no piece in this board index");
        return 0;
    }
    piece->rot = rot;
    return 0;
}

/**
 * Lua: Test to see if a piece collides with a specific spot on the board.
 */
static int boardscript_test_piece(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Parameter 2: Piece name
    const char* piece_config = luaL_checkstring(L, 2);

    // Parameter 3: Position table
    vec2i_t pos = { 0, 0 };
    bool ok = script_to_vector(L, 3, &pos);
    luaL_argcheck(L, ok, 3, "invalid position");

    // Parameter 4: Rotation integer
    lua_Integer rot = luaL_checkinteger(L, 4);

    // Internal State 1: Prototype hash
    int type = lua_getfield(L, lua_upvalueindex(1), "proto_hash");
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
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

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

    // Internal State 1: Prototype hash
    int type = lua_getfield(L, lua_upvalueindex(1), "proto_hash");
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
 * Lua: Lock a piece in place on the board.
 */
static int boardscript_lock_piece(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Parameter 2: Piece name
    const char* piece_config = luaL_checkstring(L, 2);

    // Parameter 3: Position table
    vec2i_t pos = { 0, 0 };
    bool ok = script_to_vector(L, 3, &pos);
    luaL_argcheck(L, ok, 3, "invalid position");

    // Parameter 4: Rotation integer
    lua_Integer rot = luaL_checkinteger(L, 4);

    // Internal State 1: Prototype hash
    int type = lua_getfield(L, lua_upvalueindex(1), "proto_hash");
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
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_BOARD);
    board_t* board = entity->data;

    // Clear lines and return the number of lines cleared.
    uint8_t lines = board_clear_lines(board);
    lua_pushinteger(L, lines);
    return 1;
}

int boardscript_openlib(lua_State* L) {
    static const luaL_Reg boardlib[] = {
        { "create", boardscript_create },
        { "get", boardscript_get },
        { "get_piece", boardscript_get_piece },
        { "set_piece", boardscript_set_piece },
        { "unset_piece", boardscript_unset_piece },
        { "get_pos", boardscript_get_pos },
        { "set_pos", boardscript_set_pos },
        { "get_rot", boardscript_get_rot },
        { "set_rot", boardscript_set_rot },
        { "test_piece", boardscript_test_piece },
        { "test_piece_between", boardscript_test_piece_between },
        { "lock_piece", boardscript_lock_piece },
        { "clear_lines", boardscript_clear_lines },
        { NULL, NULL }
    };

    luaL_newlib(L, boardlib);
    return 1;
}
