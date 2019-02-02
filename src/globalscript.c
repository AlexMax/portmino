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

/**
 * Much of this is copied from lbaselib from Lua 5.3.5, with some omissions
 * and changes based on the needs of the program.
 */

#include <stdlib.h>

#include "lauxlib.h"

#include "vfs.h"

/**
 * ipairs traversal function
 */
static int ipairsaux(lua_State *L) {
    lua_Integer i = luaL_checkinteger(L, 2) + 1;
    lua_pushinteger(L, i);
    return (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}

/**
 * Lua: "ipairs" function.
 */
static int globalscript_ipairs(lua_State *L) {
    luaL_checkany(L, 1);
    lua_pushcfunction(L, ipairsaux);  /* iteration function */
    lua_pushvalue(L, 1);  /* state */
    lua_pushinteger(L, 0);  /* initial value */
    return 3;
}

/**
 * Lua: "next" function.
 */
static int globalscript_next(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
    if (lua_next(L, 1)) {
        return 2;
    } else {
        lua_pushnil(L);
        return 1;
    }
}

/**
 * "pairs" traversal function.
 */
static int pairsmeta(lua_State *L, const char *method, int iszero,
                     lua_CFunction iter) {
    luaL_checkany(L, 1);
    if (luaL_getmetafield(L, 1, method) == LUA_TNIL) {  /* no metamethod? */
        lua_pushcfunction(L, iter);  /* will return generator, */
        lua_pushvalue(L, 1);  /* state, */
        if (iszero) {
            lua_pushinteger(L, 0);  /* and initial value */
        } else {
            lua_pushnil(L);
        } 
    } else {
        lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
        lua_call(L, 1, 3);  /* get 3 values from metamethod */
    }
    return 3;
}

/**
 * Lua: "pairs" function.
 */
static int globalscript_pairs(lua_State *L) {
    return pairsmeta(L, "__pairs", 0, globalscript_next);
}

/**
 * Lua: "print" function.
 */
static int globalscript_print(lua_State *L) {
    int n = lua_gettop(L);  /* number of arguments */
    lua_getglobal(L, "tostring");
    for (int i = 1;i <= n;i++) {
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        size_t l;
        const char *s = lua_tolstring(L, -1, &l);  /* get result */
        if (s == NULL) {
            return luaL_error(L, "'tostring' must return a string to 'print'");
        }
        if (i > 1) {
            lua_writestring("\t", 1);
        }
        lua_writestring(s, l);
        lua_pop(L, 1);  /* pop result */
    }
    lua_writeline();
    return 0;
}

/**
 * "require" error checking function.
 */
static int globalscript_require_error(lua_State *L, const char *filename) {
    return luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                      lua_tostring(L, 1), filename, lua_tostring(L, -1));
}

/**
 * Lua: "require" function.
 * 
 * This is actually a super-stripped down version of "require" that is missing
 * many features.  It is aware of the current environment, and package.loaded
 * contains the module itself as opposed to simply being a reference to it.
 */
static int globalscript_require(lua_State* L) {
    // Parameter 1: Name of the package.
    const char* name = luaL_checkstring(L, 1);

    // Get the current environment
    int type = lua_getfield(L, LUA_REGISTRYINDEX, "env");
    if (type != LUA_TTABLE) {
        luaL_argerror(L, 1, "require is missing internal state");
        return 0;
    }

    // Check for the existence of module in package.loaded.
    type = lua_getfield(L, -1 , "package");
    if (type != LUA_TTABLE) {
        luaL_error(L, "can't find \"package\" table");
        return 0;
    }
    type = lua_getfield(L, -1, "loaded");
    if (type != LUA_TTABLE) {
        luaL_error(L, "can't find \"loaded\" table in \"package\" table");
        return 0;
    }
    lua_getfield(L, -1, name);
    if (lua_toboolean(L, -1)) {
        // We found it, so return it!
        return 1;
    }
    lua_pop(L, 1); // pop the nil

    // Construct a filename to load from the virtual filesystem.
    // NOTE: This path construction is safe - PhysFS has no concept of ".."
    //       and symbolic links will not be traversed without our approval.
    char* filename;
    int bytes = asprintf(&filename, "ruleset/default/%s.lua", name);
    if (bytes < 0) {
        luaL_error(L, "allocation error inside require");
        return 0;
    }

    // Try and load the Lua file.
    buffer_t* file = vfs_file(filename);
    free(filename);
    if (file == NULL) {
        lua_pushstring(L, "file not found");
        // never returns
        globalscript_require_error(L, filename);
        return 0;
    }

    // Turn the buffer into a runnable chunk.
    if (luaL_loadbufferx(L, (char*)file->data, file->size, name, "t") != LUA_OK) {
        buffer_delete(file);
        // never returns
        globalscript_require_error(L, filename);
        return 0;
    }
    buffer_delete(file);

    // Call it.  Result is on the stack.  Errors are propagated back through Lua.
    lua_call(L, 0, 1);

    // Save our result to the loaded array.
    if (!lua_isnil(L, -1)) {
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, name); // pop dupe return value
    } else {
        lua_pop(L, 1); // discard nil result
        lua_pushboolean(L, 1); // our return value is "true" instead
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, name); // pop dupe return value
    }

    return 1;
}

/**
 * Lua: "tostring" function.
 */
static int globalscript_tostring(lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, NULL);
  return 1;
}

/**
 * Initialize the global module.
 */
int globalscript_openlib(lua_State* L) {
    static const luaL_Reg globallib[] = {
        { "ipairs", globalscript_ipairs },
        { "next", globalscript_next },
        { "pairs", globalscript_pairs },
        { "print", globalscript_print },
        { "require", globalscript_require },
        { "tostring", globalscript_tostring },
        { NULL, NULL }
    };

    // open lib into global table
    lua_pushglobaltable(L);
    luaL_setfuncs(L, globallib, 0);

    // set global _G
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "_G");

    // set global _VERSION
    lua_pushliteral(L, LUA_VERSION);
    lua_setfield(L, -2, "_VERSION");

    return 1;
}
