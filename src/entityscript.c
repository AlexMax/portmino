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

#include "entityscript.h"

#include "lauxlib.h"

/**
 * Lua: Destroy an entity
 */
static int entityscript_destroy(lua_State* L) {
    // Parameter 1: Our userdata
    entity_t* entity = entityscript_to_entity(L, 1, MINO_ENTITY_ANY);

    entity_deinit(entity);
    entity = NULL;

    return 0;
}

/**
 * Create entity library
 */
int entityscript_openlib(lua_State* L) {
    static const luaL_Reg entitylib[] = {
        { "destroy", entityscript_destroy },
        { NULL, NULL }
    };

    luaL_newlib(L, entitylib);
    return 1;
}

/**
 * Convert the handle at the given index to an entity
 */
entity_t* entityscript_to_entity(lua_State* L, int handle_index, entity_type_t expected_type) {
    int top = lua_gettop(L);

    // Internal State: Entity manager
    int type = lua_getfield(L, lua_upvalueindex(1), "entity_manager");
    if (type != LUA_TLIGHTUSERDATA) {
        luaL_error(L, "missing internal state (entity_manager)");
        return 0;
    }
    entity_manager_t* manager = lua_touserdata(L, -1);

    // Parameter: Our handle
    handle_t* id = luaL_checkudata(L, handle_index, "handle_t");
    entity_t* entity = entity_manager_get(manager, *id);
    if (entity == NULL) {
        luaL_error(L, "entity not found");
        return NULL;
    } else if (entity->config.type != MINO_ENTITY_ANY && entity->config.type != expected_type) {
        luaL_error(L, "entity is not of the correct type");
        return NULL;
    }

    lua_settop(L, top);
    return entity;
}
