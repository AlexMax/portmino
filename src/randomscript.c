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

#include "entity.h"
#include "random.h"

#include "script.h"

/**
 * Lua: Initialize new random state.
 */
static int randomscript_create(lua_State* L) {
    // Parameter 1: Seed integer (or nil if we want a truly random seed)
    int seed_type = lua_type(L, 1);
    luaL_argcheck(L, (seed_type == LUA_TNUMBER || seed_type == LUA_TNIL), 1, "invalid seed");

    // Internal State 1: Registry reference
    int type = lua_getfield(L, lua_upvalueindex(1), "registry_ref");
    if (type != LUA_TNUMBER) {
        luaL_error(L, "missing internal state (registry_ref)");
        return 0;
    }

    // Internal State 2: Entity manager
    type = lua_getfield(L, lua_upvalueindex(1), "entity_manager");
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
    bool ok;
    if (seed_type == LUA_TNUMBER) {
        uint32_t state = (uint32_t)lua_tointeger(L, 1);
        ok = random_entity_init(entity, &state);
    } else {
        ok = random_entity_init(entity, NULL);
    }
    if (ok == false) {
        luaL_error(L, "could not initialize entity");
        return 0;
    }

    // Allocate a handle for our entity
    uint64_t* id = lua_newuserdata(L, sizeof(uint64_t));
    *id = entity->id;

    // Designate as an entity handle
    luaL_setmetatable(L, "handle_t");
    return 1;
}

/**
 * Lua: Get a random number with a maximum value
 */
static int randomscript_number(lua_State* L) {
    // Internal State 1: Entity manager
    int type = lua_getfield(L, lua_upvalueindex(1), "entity_manager");
    if (type != LUA_TLIGHTUSERDATA) {
        luaL_error(L, "missing internal state (entity_manager)");
        return 0;
    }
    entity_manager_t* manager = lua_touserdata(L, -1);

    // Parameter 1: Our handle
    uint64_t* id = luaL_checkudata(L, 1, "handle_t");
    entity_t* entity = entity_manager_get(manager, *id);
    if (entity == NULL) {
        luaL_error(L, "entity not found");
        return 0;
    } else if (entity->config.type != MINO_ENTITY_RANDOM) {
        luaL_error(L, "entity is not a random entity");
        return 0;
    }
    random_t* random = entity->data;

    // Parameter 2: Integer random number range
    lua_Integer range = luaL_checkinteger(L, 2);

    // Actually return the random number.
    uint32_t number = random_number(random, (uint32_t)range);
    lua_pushinteger(L, number);
    return 1;
}

/**
 * Initialize the random module.
 */
int randomscript_openlib(lua_State* L) {
    static const luaL_Reg randomlib[] = {
        { "create", randomscript_create },
        { "number", randomscript_number },
        { NULL, NULL }
    };

    luaL_newlib(L, randomlib);
    return 1;
}
