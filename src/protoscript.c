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

#include "define.h"

#include "lua.h"
#include "lauxlib.h"

/**
 * Lua: Load a prototype.
 */
int protoscript_load(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    // Internal State 1: protos table
    int type = lua_getfield(L, lua_upvalueindex(1), "proto_hash");
    if (type != LUA_TTABLE) {
        luaL_error(L, "require: missing internal state (proto_hash)");
        return 0;
    }

    // Internal State 2: protos container
    type = lua_getfield(L, lua_upvalueindex(1), "proto_container");
    if (type != LUA_TLIGHTUSERDATA) {
        luaL_error(L, "require: missing internal state (proto_container)");
        return 0;
    }

    return 0;
}

int protoscript_openlib(lua_State* L) {
    static const luaL_Reg protolib[] = {
        { "load", protoscript_load },
        { NULL, NULL }
    };

    luaL_newlib(L, protolib);
    return 1;
}
