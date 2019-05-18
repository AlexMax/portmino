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

/**
 * Lua: Initialize new random state.
 */
static int randomscript_new(lua_State* L) {
    // Parameter 1: Seed integer (or nil if we want a truly random seed)
    int seed_type = lua_type(L, 1);
    luaL_argcheck(L, (seed_type == LUA_TNUMBER || seed_type == LUA_TNIL), 1, "invalid seed");

    // Internal State 1: Registry reference
    int type = lua_getfield(L, lua_upvalueindex(1), "registry_ref");
    if (type != LUA_TNUMBER) {
        luaL_error(L, "missing internal state (registry_ref)");
        return 0;
    }

    // Internal State 2: Next ID
    type = lua_getfield(L, lua_upvalueindex(1), "entity_next");
    if (type != LUA_TNUMBER) {
        luaL_error(L, "missing internal state (entity_next)");
        return 0;
    }

    uint32_t entity_next = lua_tointeger(L, -1);
    int registry_ref = lua_tointeger(L, -2);

    // Initialize (and return) random state
    entity_t* entity = lua_newuserdata(L, sizeof(entity_t));

    random_t* random;
    if (seed_type == LUA_TNUMBER) {
        uint32_t seed = (uint32_t)lua_tointeger(L, 1);
        random = random_new(&seed);
    } else {
        random = random_new(NULL);
    }
    if (random == NULL) {
        luaL_error(L, "Could not allocate new random state.");
        return 0;
    }

    // Set our entity properties
    entity->id = entity_next;
    entity->registry_ref = registry_ref;
    entity->type = MINO_ENTITY_RANDOM;
    entity->data = random;
    entity->destruct = random_destruct;

    // Apply methods to the userdata
    luaL_setmetatable(L, "random_t");

    // Increment our next entity counter
    lua_pushinteger(L, entity_next + 1);
    lua_setfield(L, lua_upvalueindex(1), "entity_next");

    return 1;
}

/**
 * Lua: random_t finalizer
 */
static int randomscript_delete(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = luaL_checkudata(L, 1, "random_t");

    entity_deinit(entity);
    entity = NULL;

    return 0;
}

/**
 * Lua: Get a random number with a maximum value
 */
static int randomscript_number(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = luaL_checkudata(L, 1, "random_t");
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
        { "new", randomscript_new },
        { NULL, NULL }
    };

    luaL_newlib(L, randomlib);

    // Create the random_t type
    static const luaL_Reg randomtype[] = {
        { "__gc", randomscript_delete },
        { "number", randomscript_number },
        { NULL, NULL }
    };

    luaL_newmetatable(L, "random_t");
    luaL_newlib(L, randomtype);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    return 1;
}
