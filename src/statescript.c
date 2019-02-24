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

#include "state.h"

#include "lua.h"
#include "lauxlib.h"

/**
 * Lua: Get the current gametic of the state
 */
static int statescript_get_gametic(lua_State* L) {
    lua_pushstring(L, "state");
    lua_gettable(L, LUA_REGISTRYINDEX);

    const state_t* state = lua_touserdata(L, -1);
    if (state == NULL) {
        // never returns
        luaL_error(L, "statescript_get_gametic is missing internal state");
    }

    lua_pushnumber(L, state->tic);
    return 1;
}

/**
 * Push library functions into the state.
 */
int statescript_openlib(lua_State* L) {
    static const luaL_Reg statelib[] = {
        { "get_gametic", statescript_get_gametic },
        { NULL, NULL }
    };

    luaL_newlib(L, statelib);

    return 1;
}
