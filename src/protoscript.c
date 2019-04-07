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

#include "piece.h"
#include "proto.h"
#include "script.h"

/**
 * Lua: Load a prototype.
 */
int protoscript_load(lua_State* L) {
    static const char* types[] = {
        "piece", NULL
    };

    // Parameter 1: prototype type
    int option = luaL_checkoption(L, 1, NULL, types);

    // Parameter 2: prototype name
    const char* name = luaL_checkstring(L, 2);

    // Parameter 3: prototype data
    luaL_checktype(L, 3, LUA_TTABLE);

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
    proto_container_t* protos = lua_touserdata(L, -1);

    // First check to see if we're pushing a duplicate prototype
    if (lua_getfield(L, -2, name) != LUA_TNIL) {
        luaL_error(L, "require: prototype \"%s\" already exists", name);
        return 0;
    }
    lua_pop(L, 1); // pop nil

    lua_pushvalue(L, -3); // push dupe configuration

    // Depending on our prototype type, create a different prototype
    proto_t* proto = NULL;
    switch (option) {
    case 0: {
        piece_config_t* piece = piece_config_new(L, name); // pops config
        if (piece == NULL) {
            luaL_error(L, "require: could not create piece:\n\t%s", lua_tostring(L, -1));
            return 0;
        }

        proto = proto_new(PROTO_PIECE, piece, piece_config_destruct);
        if (proto == NULL) {
            piece_config_delete(piece);
            luaL_error(L, "require: could not create prototype");
            return 0;
        }

        lua_pushlightuserdata(L, piece);
        break;
    }
    default:
        luaL_argerror(L, 1, "require: unknown type");
        return 0;
    }

    if (proto_container_push(protos, proto) == false) {
        luaL_error(L, "require: could not add prototype to container");
        return 0;
    }

    // At this point, nothing else can go wrong, so we can push our pointer
    // into proto_hash without worrying that it doesn't work.
    lua_setfield(L, -3, name);

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
