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

#include "script.h"

#include <stdio.h>
#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "audioscript.h"
#include "board.h"
#include "boardscript.h"
#include "error.h"
#include "inputscript.h"
#include "globalscript.h"
#include "piece.h"
#include "piecescript.h"
#include "protoscript.h"
#include "random.h"
#include "randomscript.h"
#include "renderscript.h"
#include "vfs.h"

/**
 * Create a lua state that contains all of our libraries
 */
lua_State* script_newstate(void) {
    lua_State* L = NULL;

    // Create a new Lua state
    if ((L = luaL_newstate()) == NULL) {
        error_push_allocerr();
        return NULL;
    }

    static const luaL_Reg loadedlibs[] = {
        { "_G", globalscript_openlib },
        { "mino_audio", audioscript_openlib },
        { "mino_board", boardscript_openlib },
        { "mino_input", inputscript_openlib },
        { "mino_piece", piecescript_openlib },
        { "mino_proto", protoscript_openlib },
        { "mino_random", randomscript_openlib },
        { "mino_render", renderscript_openlib },
        { LUA_MATHLIBNAME, luaopen_math },
        { LUA_STRLIBNAME, luaopen_string },
        { LUA_TABLIBNAME, luaopen_table },
        { NULL, NULL }
    };

    // Push our library functions into the new state.
    for (const luaL_Reg* lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);
    }

    return L;
}

/**
 * Turn the table at the given stack index into a vector.
 */
bool script_to_vector(lua_State* L, int index, vec2i_t* vec) {
    if (lua_type(L, index) != LUA_TTABLE) {
        return false;
    }

    lua_getfield(L, index, "x");
    vec->x = lua_tointeger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, index, "y");
    vec->y = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return true;
}

/**
 * Push a table to the stack with the contents of the given vector.
 */
void script_push_vector(lua_State* L, const vec2i_t* vec) {
    lua_createtable(L, 2, 0);
    lua_pushinteger(L, vec->x);
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, vec->y);
    lua_setfield(L, -2, "y");
}

/**
 * Recursively wrap all cfunctions with closures containing the given index
 * as a first upvalue
 *
 * Pops the unwrapped item from the top of the stack and pushes a wrapped
 * version of the item.  If the item didn't need to be wrapped, nothing
 * happens.
 */
void script_wrap_cfuncs(lua_State* L, int index) {
    if (index < 0) {
        // First translate into absolute index
        index = lua_gettop(L) + index + 1;
    }

    switch (lua_type(L, -1)) {
    case LUA_TTABLE:
        lua_createtable(L, 0, 0); // push dest table
        lua_pushnil(L); // push initial key
        while (lua_next(L, -3) != 0) {
            script_wrap_cfuncs(L, index); // recursively replace table value
            lua_pushvalue(L, -2); // dupe key
            lua_insert(L, -2); // move dupe key to proper position
            lua_settable(L, -4); // pop dupe key and value
        }
        lua_remove(L, -2); // remove original table
        break;
    case LUA_TFUNCTION: {
        lua_CFunction func = lua_tocfunction(L, -1);
        if (func != NULL) {
            lua_pop(L, 1); // pop cfunction
            lua_pushvalue(L, index); // push registry
            lua_pushcclosure(L, func, 1); // pop registry, push closure
        }
        break;
    }
    default:
        // No need to wrap anything
        break;
    }
}

/**
 * Take a configuration file and push a table that contains all defined
 * configuration values to the stack, or an error message if there was a
 * problem parsing the file
 * 
 * Configuration files are just Lua scripts run in the context of an empty
 * environment.
 */
bool script_load_config(lua_State* L, vfile_t* file) {
    // Load the file into Lua as a chunk.
    if (luaL_loadbufferx(L, (char*)file->data, file->size, file->filename, "t") != LUA_OK) {
        // We have an error on the stack, just return it.
        return false;
    }

    // Our environment is an empty table
    lua_newtable(L);
    lua_pushvalue(L, -1); // copy env ptr
    lua_setupvalue(L, -3, 1); // set environment to env pointer, pops env ptr

    // Move the chunk to the top of the stack and call it.
    lua_insert(L, -2);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        // Right now we have an error message _and_ our useless environment
        // pointer.  Get rid of the environment and return just the error.
        lua_insert(L, -1);
        lua_pop(L, 1);
        return false;
    }

    // We have the pointer to our environment at the top of the stack.
    return true;
}

/**
 * Set our package search paths based on our current ruleset and gametype
 * 
 * Note that gametype can be NULL, and both gametype and ruleset can be NULL.
 */
void script_push_paths(lua_State* L, const char* ruleset, const char* gametype) {
    if (gametype != NULL) {
        // Our search path contains gametype and ruleset
        lua_pushfstring(L, 
            "gametype/%s/%s/?.lua;" "gametype/%s/%s/?/init.lua;" \
            "ruleset/%s/?.lua;" "ruleset/%s/?/init.lua;" \
            "script/?.lua;" "script/?/init.lua",
            ruleset, gametype, ruleset, gametype, ruleset, ruleset);
    } else if (ruleset != NULL) {
        // Our search path contains ruleset
        lua_pushfstring(L, 
            "ruleset/%s/?.lua;" "ruleset/%s/?/init.lua;" \
            "script/?.lua;" "script/?/init.lua",
            ruleset, ruleset);
    } else {
        // Our search path just has script information
        lua_pushstring(L, "script/?.lua;" "script/?/init.lua");
    }
}

/**
 * Set our config (not C library) search paths based on our current
 * ruleset and gametype
 * 
 * Note that gametype can be NULL, and both gametype and ruleset can be NULL.
 */
void script_push_cpaths(lua_State* L, const char* ruleset, const char* gametype) {
    if (gametype != NULL) {
        // Our search path contains gametype and ruleset
        lua_pushfstring(L,
            "gametype/%s/?.cfg;" "ruleset/%s/?.cfg;" "script/?.cfg;",
            gametype, ruleset);
    } else if (ruleset != NULL) {
        // Our search path contains ruleset
        lua_pushfstring(L,
            "ruleset/%s/?.cfg;" "script/?.cfg", ruleset);
    } else {
        // Our search path just has script information
        lua_pushstring(L, "script/?.cfg");
    }
}

/**
 * Dump the contents of a specific stack index
 */
void script_debug(lua_State* L, int index) {
    int type = lua_type(L, index);
    fprintf(stderr, "(%s) ", lua_typename(L, type));
    switch (type) {
    case LUA_TNIL:
        fprintf(stderr, "nil");
        break;
    case LUA_TBOOLEAN:
        fprintf(stderr, "%s", lua_toboolean(L, index) ? "true" : "false");
        break;
    case LUA_TLIGHTUSERDATA:
    case LUA_TUSERDATA:
        fprintf(stderr, "%p", lua_touserdata(L, index));
        break;
    case LUA_TNUMBER:
        fprintf(stderr, "%f", lua_tonumber(L, index));
        break;
    case LUA_TSTRING:
        fprintf(stderr, "%s", lua_tostring(L, index));
        break;
    default:
        break;
    }
}

/**
 * Dump the contents of the stack
 */
void script_debug_stack(lua_State* L) {
    int top = lua_gettop(L);
    if (top == 0) {
        fprintf(stderr, "Stack is empty...\n");
        return;
    }

    for (int i = 1;i <= top;i++) {
        fprintf(stderr, "[%d] ", i);
        script_debug(L, i);
        fprintf(stderr, "\n");
    }
}

/**
 * Dump the contents of a table at a specific index
 */
void script_debug_table(lua_State* L, int index) {
    if (index < 0) {
        // First translate into absolute index
        index = lua_gettop(L) + index + 1;
    }

    if (lua_type(L, index) != LUA_TTABLE) {
        fputs("not a table\n", stderr);
        return;
    }

    bool empty = true;
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
        empty = false;
        script_debug(L, -2); // key
        fputs(", ", stderr);
        script_debug(L, -1); // value
        fputs("\n", stderr);
        lua_pop(L, 1); // keep the key around
    }

    if (empty == true) {
        fputs("empty table\n", stderr);
        return;
    }
}
