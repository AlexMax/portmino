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
#include <string.h>

#include "lauxlib.h"

#include "error.h"
#include "script.h"
#include "vfs.h"

 /**
  * Find the next search path
  */
static const char* pushnexttemplate(lua_State* L, const char* path) {
    while (*path == *LUA_PATH_SEP) {
        path++;  /* skip separators */
    }
    if (*path == '\0') {
        return NULL;  /* no more templates */
    }
    const char* l = strchr(path, *LUA_PATH_SEP);  /* find next separator */
    if (l == NULL) {
        l = path + strlen(path);
    }
    lua_pushlstring(L, path, l - path);  /* template */
    return l;
}

/**
 * Check to see if a file exists anywhere in the given search path
 */
static vfile_t* searchpath(lua_State* L, const char* name, const char* path) {
    luaL_Buffer msg;
    luaL_buffinit(L, &msg);

    while ((path = pushnexttemplate(L, path)) != NULL) {
        const char* filename = luaL_gsub(L, lua_tostring(L, -1),
            LUA_PATH_MARK, name);
        lua_remove(L, -2);  // pop path template

        vfile_t* file = vfs_vfile_new(filename);
        if (file != NULL) {
            lua_pop(L, 1); // pop filename
            return file;
        }

        // Append to our error message
        lua_pushfstring(L, "\n\tno file '%s'", filename);
        lua_remove(L, -2);  // pop filename
        luaL_addvalue(&msg);
    }

    // Push our error message
    luaL_pushresult(&msg);
    return NULL;
}

/**
 * Lua: Load a configuration file, execute it, and return any globals
 *      that a config sets in its environment
 */
static int globalscript_doconfig(lua_State* L) {
    vfile_t* file = NULL;

    // Parameter 1: Name of the package.
    const char* name = luaL_checkstring(L, 1);

    // Upvalue 1: Search paths
    lua_pushvalue(L, lua_upvalueindex(1));
    if (lua_type(L, -1) != LUA_TSTRING) {
        luaL_error(L, "missing internal state");
        return 0;
    }

    // Load our search paths
    const char* paths = lua_tostring(L, -1);

    // Try and load the Lua file from our search paths
    if ((file = searchpath(L, name, paths)) == NULL) {
        // searchpath pushes error string on failure
        goto fail;
    }

    // Load the file as a configuration file
    if (script_load_config(L, file) == false) {
        // pushes error string on failure
        goto fail;
    };

    vfs_vfile_delete(file);
    return 1;

fail:
    vfs_vfile_delete(file);
    luaL_error(L, "config '%s' not found:\n\t%s",
        lua_tostring(L, 1), lua_tostring(L, -1));
    return 0;
}

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
 * Lua: "require" function.
 * 
 * This is actually a super-stripped down version of "require" that is missing
 * many features.  The environment to load the module against is stored in
 * the first upvalue, loaded packages are stored in the second upvalue, and
 * package search paths are stored in the third.
 */
static int globalscript_require(lua_State* L) {
    vfile_t* file = NULL;

    // Parameter 1: Name of the package.
    const char* name = luaL_checkstring(L, 1);

    // Upvalue 1: _ENV
    lua_pushvalue(L, lua_upvalueindex(1));
    if (lua_type(L, -1) != LUA_TTABLE) {
        luaL_error(L, "missing internal state");
        return 0;
    }

    // Upvalue 2: Loaded modules
    lua_pushvalue(L, lua_upvalueindex(2));
    if (lua_type(L, -1) != LUA_TTABLE) {
        luaL_error(L, "missing internal state");
        return 0;
    }

    // Upvalue 3: Search paths
    lua_pushvalue(L, lua_upvalueindex(3));
    if (lua_type(L, -1) != LUA_TSTRING) {
        luaL_error(L, "missing internal state");
        return 0;
    }

    // Check for the existence of module
    lua_getfield(L, -2, name);
    if (lua_toboolean(L, -1)) {
        // We found it, so return it!
        return 1;
    }
    lua_pop(L, 1); // pop the nil

    // Load our search paths
    const char* paths = lua_tostring(L, -1);

    // Try and load the Lua file from our search paths
    if ((file = searchpath(L, name, paths)) == NULL) {
        // searchpath pushes error string on failure
        goto fail;
    }

    // Turn the buffer into a runnable chunk.
    if (luaL_loadbufferx(L, (char*)file->data, file->size, file->filename, "t") != LUA_OK) {
        lua_pushstring(L, name);
        goto fail;
    }

    // We don't need the file anymore.
    vfs_vfile_delete(file);
    file = NULL;

    // Set the environment of the chunk
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_setupvalue(L, -2, 1);

    // Call it.  Result is on the stack.  Errors are propagated back through Lua.
    lua_call(L, 0, 1);

    // Save our result to the loaded array.
    if (!lua_isnil(L, -1)) {
        lua_pushvalue(L, -1);
        lua_setfield(L, -4, name); // pop dupe return value
    } else {
        lua_pop(L, 1); // discard nil result
        lua_pushboolean(L, 1); // our return value is "true" instead
        lua_pushvalue(L, -1);
        lua_setfield(L, -4, name); // pop dupe return value
    }

    return 1;

fail:
    vfs_vfile_delete(file);
    luaL_error(L, "module '%s' not found:\n\t%s",
        lua_tostring(L, 1), lua_tostring(L, -1));
    return 0;
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
        { "doconfig", globalscript_doconfig },
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
