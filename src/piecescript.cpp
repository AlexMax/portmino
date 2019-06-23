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

#include "lua.hpp"

#include "entity.hpp"
#include "piece.hpp"
#include "proto.hpp"
#include "script.hpp"

/**
 * Lua: Initialize new board state.
 */
static int piecescript_new(lua_State* L) {
    // Parameter 1: Piece configuration
    const char* piece_config = luaL_checkstring(L, 1);

    // Internal State 1: Prototype hash
    int type = lua_getfield(L, lua_upvalueindex(1), "proto_hash");
    if (type != LUA_TTABLE) {
        luaL_error(L, "missing internal state (proto_hash)");
        return 0;
    }

    // Internal State 2: Registry ref
    type = lua_getfield(L, lua_upvalueindex(1), "registry_ref");
    if (type != LUA_TNUMBER) {
        luaL_error(L, "missing internal state (registry_ref)");
        return 0;
    }

    // Internal State 3: Next ID
    type = lua_getfield(L, lua_upvalueindex(1), "entity_next");
    if (type != LUA_TNUMBER) {
        luaL_error(L, "missing internal state (entity_next)");
        return 0;
    }

    // Get our entity data
    uint32_t entity_next = lua_tointeger(L, -1);
    int registry_ref = lua_tointeger(L, -2);

    // Get the piece configuration
    lua_getfield(L, -3, piece_config);
    proto_t* proto = lua_touserdata(L, -1);
    if (proto == NULL || proto->type != MINO_PROTO_PIECE) {
        luaL_error(L, "invalid piece configuration");
        return 0;
    }
    piece_config_t* config = proto->data;

    // Initialize (and return) board state
    entity_t* entity = lua_newuserdata(L, sizeof(entity_t));
    piece_entity_init(entity);
    if ((entity->data = piece_new(config)) == NULL) {
        luaL_error(L, "Could not allocate new piece.");
        return 0;
    }

    // Set our entity properties
    entity->id = entity_next;
    entity->registry_ref = registry_ref;

    // Apply methods to the entity
    luaL_setmetatable(L, "piece_t");

    // Increment our next entity counter
    lua_pushinteger(L, entity_next + 1);
    lua_setfield(L, lua_upvalueindex(1), "entity_next");

    return 1;
}

/**
 * Lua: board_t finalizer.
 */
static int piecescript_delete(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = luaL_checkudata(L, 1, "piece_t");

    entity_deinit(entity);
    entity = NULL;

    return 0;
}

/**
 * Lua: Get the configuration userdata for a given piece handle.
 */
static int piecescript_config_name(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = luaL_checkudata(L, 1, "piece_t");
    piece_t* piece = entity->data;

    lua_pushstring(L, piece->config->name);
    return 1;
}

/**
 * Lua: Get the number of rotations that a piece has.
 */
static int piecescript_config_spawn_pos(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = luaL_checkudata(L, 1, "piece_t");
    piece_t* piece = entity->data;

    script_push_vector(L, &piece->config->spawn_pos);
    return 1;
}

/**
 * Lua: Get the number of rotations that a piece has.
 */
static int piecescript_config_spawn_rot(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = luaL_checkudata(L, 1, "piece_t");
    piece_t* piece = entity->data;

    lua_pushinteger(L, piece->config->spawn_rot);
    return 1;
}

/**
 * Lua: Get the number of rotations that a piece has.
 */
static int piecescript_config_rot_count(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = luaL_checkudata(L, 1, "piece_t");
    piece_t* piece = entity->data;

    lua_pushinteger(L, piece->config->data_count);
    return 1;
}

/**
 * Push library functions into the state.
 */
int piecescript_openlib(lua_State* L) {
    static const luaL_Reg piecelib[] = {
        { "new", piecescript_new },
        { NULL, NULL }
    };

    luaL_newlib(L, piecelib);

    // Create the piece_t type
    static const luaL_Reg piece_meta[] = {
        { "__gc", piecescript_delete },
        { NULL, NULL }
    };
    static const luaL_Reg piece_methods[] = {
        { "config_name", piecescript_config_name },
        { "config_spawn_pos", piecescript_config_spawn_pos },
        { "config_spawn_rot", piecescript_config_spawn_rot },
        { "config_rot_count", piecescript_config_rot_count },
        { NULL, NULL }
    };

    luaL_newmetatable(L, "piece_t"); // push meta
    luaL_setfuncs(L, piece_meta, 0);
    luaL_newlib(L, piece_methods); // push methods table
    lua_setfield(L, -2, "__index"); // pop methods table
    lua_pop(L, 1); // pop meta

    return 1;
}
