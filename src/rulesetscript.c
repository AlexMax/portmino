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

#include "gametype.h"
#include "ruleset.h"
#include "script.h"
#include "state.h"

/**
 * Lua: Call a gametype function from the ruleset
 */
static int rulesetscript_gametype_call(lua_State* L) {
    // Parameter 1: Function name
    const char* func = luaL_checkstring(L, 1);

    // Subsequent parameters are parameters to pass to the function
    int params = lua_gettop(L) - 1;

    // Grab our gametype pointer out of the registry
    lua_getfield(L, LUA_REGISTRYINDEX, "gametype");
    const gametype_t* gametype = lua_touserdata(L, -1);
    if (gametype == NULL) {
        luaL_error(L, "rulesetscript_gametype_call is missing internal state");
        return 0;
    }

    // Grab a reference to our state function table
    lua_rawgeti(L, LUA_REGISTRYINDEX, gametype->state_functions_ref);

    // Grab our state function with the given name
    int type = lua_getfield(L, -1, func);

    if (type == LUA_TFUNCTION) {
        // Call the gametype state function, and return all params.
        lua_replace(L, 1);
        lua_settop(L, params + 1);
        lua_pcall(L, params, LUA_MULTRET, 0);
        return lua_gettop(L);
    } else if (type == LUA_TNIL) {
        lua_pushnil(L);
        return 1;
    }

    luaL_error(L, "Gametype state function \"%s\" is not function or nil.");
    return 0;
}

/**
 * Lua: Return a list of piece configs attached to the state.
 */
static int rulesetscript_get_piece_configs(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "ruleset");
    const ruleset_t* ruleset = lua_touserdata(L, -1);
    if (ruleset == NULL) {
        luaL_error(L, "ruleset_get_piece_configs is missing internal state");
        return 0;
    }

    // Fetch the pieces and return an array of configs as a table.
    lua_createtable(L, ruleset->pieces->size, 0);
    for (size_t i = 1;i <= ruleset->pieces->size;i++) {
        // Lua is 1-indexed, but C is 0.
        lua_pushlightuserdata(L, ruleset->pieces->configs[i - 1]);
        lua_rawseti(L, -2, i);
    }

    return 1;
}

/**
 * Push library functions into the state.
 */
int rulesetscript_openlib(lua_State* L) {
    static const luaL_Reg rulesetlib[] = {
        { "gametype_call", rulesetscript_gametype_call },
        { "get_piece_configs", rulesetscript_get_piece_configs },
        { NULL, NULL }
    };

    luaL_newlib(L, rulesetlib);

    return 1;
}
