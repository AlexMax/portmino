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

#include "entity.h"
#include "entityscript.h"
#include "piece.h"
#include "proto.h"
#include "script.h"

/**
 * Lua: Initialize new piece state.
 */
static int piecescript_create(lua_State* L) {
    // Parameter 1: Piece configuration
    const char* piece_config = luaL_checkstring(L, 1);

    // Internal State 1: Entity manager
    int type = lua_getfield(L, lua_upvalueindex(1), "entity_manager");
    if (type != LUA_TLIGHTUSERDATA) {
        luaL_error(L, "missing internal state (entity_manager)");
        return 0;
    }
    entity_manager_t* manager = lua_touserdata(L, -1);

    // Internal State 2: Prototype hash
    type = lua_getfield(L, lua_upvalueindex(1), "proto_hash");
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
    piece_config_t* config = proto->data;

    // Allocate the entity
    entity_t* entity = entity_manager_create(manager);
    if (entity == NULL) {
        luaL_error(L, "could not allocate new entity");
        return 0;
    }

    // Initialize the entity with random state
    bool ok = piece_entity_init(entity, config);
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
 * Lua: Get the configuration userdata for a given piece handle.
 */
static int piecescript_config_name(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_PIECE);
    piece_t* piece = entity->data;

    lua_pushstring(L, piece->config->name);
    return 1;
}

/**
 * Lua: Get the number of rotations that a piece has.
 */
static int piecescript_config_spawn_pos(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_PIECE);
    piece_t* piece = entity->data;

    script_push_vector(L, &piece->config->spawn_pos);
    return 1;
}

/**
 * Lua: Get the number of rotations that a piece has.
 */
static int piecescript_config_spawn_rot(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_PIECE);
    piece_t* piece = entity->data;

    lua_pushinteger(L, piece->config->spawn_rot);
    return 1;
}

/**
 * Lua: Get the number of rotations that a piece has.
 */
static int piecescript_config_rot_count(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_PIECE);
    piece_t* piece = entity->data;

    lua_pushinteger(L, piece->config->data_count);
    return 1;
}

/**
 * Push library functions into the state.
 */
int piecescript_openlib(lua_State* L) {
    static const luaL_Reg piecelib[] = {
        { "create", piecescript_create },
        { "config_name", piecescript_config_name },
        { "config_spawn_pos", piecescript_config_spawn_pos },
        { "config_spawn_rot", piecescript_config_spawn_rot },
        { "config_rot_count", piecescript_config_rot_count },
        { NULL, NULL }
    };

    luaL_newlib(L, piecelib);
    return 1;
}
