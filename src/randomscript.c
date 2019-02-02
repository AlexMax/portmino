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

#include "random.h"

/**
 * Lua: Initialize new random state.
 */
static int randomscript_new(lua_State* L) {
    // Parameter 1: Seed integer (or nil if we want a truly random seed)
    int type = lua_type(L, 1);
    luaL_argcheck(L, (type == LUA_TNUMBER || type == LUA_TNIL), 1, "invalid seed");

    // Initialize (and return) random state
    random_t* random = lua_newuserdata(L, sizeof(random_t));
    if (type == LUA_TNUMBER) {
        uint32_t seed = (uint32_t)lua_tointeger(L, 1);
        random_init(random, &seed);
    } else {
        random_init(random, NULL);
    }

    // Apply methods to the userdata
    luaL_setmetatable(L, "random_t");

    return 1;
}

/**
 * Lua: Get a random number with a maximum value
 */
static int randomscript_number(lua_State* L) {
    // Parameter 1: Our userdata
    random_t* random = luaL_checkudata(L, 1, "random_t");

    // Parameter 2: Integer random number range
    lua_Integer range = luaL_checkinteger(L, 2);

    // Actually product the random number.
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
        { "number", randomscript_number },
        { NULL, NULL }
    };

    luaL_newmetatable(L, "random_t");
    luaL_newlib(L, randomtype);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    return 1;
}
