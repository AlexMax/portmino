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

#include "gametype.h"

#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#include "error.h"
#include "ruleset.h"
#include "vfs.h"

/**
 * Return a buffer containing a script for a gametype of a specific name
 *
 * The buffer must be freed by the caller.
 */
buffer_t* gametype_find_script(const char* ruleset_name, const char* name) {
    char* filename = NULL;
    buffer_t* file = NULL;

    // Construct a path to the gametype.
    int ok = asprintf(&filename, "gametype/%s/%s/main.lua", ruleset_name, name);
    if (ok < 0) {
        error_push_allocerr();
        goto fail;
    }

    // Load the file with our gametype in it.
    if ((file = vfs_file(filename)) == NULL) {
        error_push("Could not find gametype %s.", name);
        goto fail;
    }

    free(filename);
    return file;

fail:
    free(filename);
    buffer_delete(file);
    return NULL;
}

/**
 * Allocate a new gametype
 */
gametype_t* gametype_new(lua_State* L, buffer_t* file, const char* name) {
    int init_ref = LUA_NOREF;
    int state_functions_ref = LUA_NOREF;
    int draw_ref = LUA_NOREF;
    gametype_t* gametype = NULL;

    // Load our file into the Lua state.
    if (luaL_loadbufferx(L, (char*)file->data, file->size, name, "t") != LUA_OK) {
        error_push("%s", lua_tostring(L, -1));
        goto fail;
    }

    // We now have the gametype on the top of the stack.  Call it!
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        error_push("lua_error: %s", lua_tostring(L, -1));
        goto fail;
    }

    // We should have a table at the top of the stack that contains the
    // functions that implement the ruleset.
    if (lua_type(L, -1) != LUA_TTABLE) {
        error_push("Could not find module table in ganetype %s.", name);
        goto fail;
    }

    // Do we have an init function?
    int type = lua_getfield(L, -1, "init");
    if (type != LUA_TFUNCTION && type != LUA_TNIL) {
        error_push("\"init\" in module table must be function or nil in gametype %s.", name);
        goto fail;
    }

    if (type != LUA_TNIL) {
        // Create a reference to init function.
        init_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        if (init_ref == LUA_REFNIL) {
            error_push_allocerr();
            goto fail;
        }
    } else {
        // Pop the useless nil.
        lua_pop(L, 1);
    }

    // Do we have a table of state functions?
    type = lua_getfield(L, -1, "state_functions");
    if (type != LUA_TTABLE && type != LUA_TNIL) {
        error_push("\"state_functions\" in module table must be table or nil in gametype %s.", name);
        goto fail;
    }

    if (type != LUA_TNIL) {
        // Create a reference to the table of functions.
        state_functions_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        if (state_functions_ref == LUA_REFNIL) {
            error_push_allocerr();
            goto fail;
        }
    } else {
        // Pop the useless nil.
        lua_pop(L, 1);
    }

    // Do we have a draw function?
    type = lua_getfield(L, -1, "draw");
    if (type != LUA_TFUNCTION && type != LUA_TNIL) {
        error_push("\"draw\" in module table must be function or nil in gametype %s.", name);
        goto fail;
    }

    if (type != LUA_TNIL) {
        // Create a reference to the draw function.
        draw_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        if (draw_ref == LUA_REFNIL) {
            error_push_allocerr();
            goto fail;
        }
    } else {
        // Pop the useless nil.
        lua_pop(L, 1);
    }

    // We should only have the gametype on the stack.  Always finish your
    // Lua meddling with a clean stack.
    if (lua_gettop(L) != 1) {
        error_push("Lua stack not cleaned up.");
        goto fail;
    }
    lua_pop(L, 1);

    // Allocate the gametype.
    if ((gametype = calloc(1, sizeof(gametype_t))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Make a copy of the name, so we can attach it.
    if ((gametype->name = strdup(name)) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    gametype->lua = L;
    gametype->init_ref = init_ref;
    gametype->state_functions_ref = state_functions_ref;
    gametype->draw_ref = draw_ref;

    return gametype;

fail:
    luaL_unref(L, LUA_REGISTRYINDEX, init_ref);
    luaL_unref(L, LUA_REGISTRYINDEX, state_functions_ref);
    luaL_unref(L, LUA_REGISTRYINDEX, draw_ref);
    gametype_delete(gametype);
    return NULL;
}

/**
 * Free the gametype
 */
void gametype_delete(gametype_t* gametype) {
    if (gametype == NULL) {
        return;
    }

    free(gametype->name);
    luaL_unref(gametype->lua, LUA_REGISTRYINDEX, gametype->init_ref);
    luaL_unref(gametype->lua, LUA_REGISTRYINDEX, gametype->state_functions_ref);
    luaL_unref(gametype->lua, LUA_REGISTRYINDEX, gametype->draw_ref);
    free(gametype);
}

/**
 * Run our initialization functions for a new game
 */
bool gametype_initialize(gametype_t* gametype, state_t* state) {
    // Ensure our state is in the registry
    lua_pushlightuserdata(gametype->lua, state);
    lua_setfield(gametype->lua, LUA_REGISTRYINDEX, "state");

    // Get our initialization function out of the registry
    int type = lua_rawgeti(gametype->lua, LUA_REGISTRYINDEX, gametype->init_ref);
    if (type == LUA_TNIL) {
        // No initilization function, we're fine.
        return true;
    } else if (type == LUA_TFUNCTION) {
        // Call the function.
        if (lua_pcall(gametype->lua, 0, 0, 0) != LUA_OK) {
            const char* err = lua_tostring(gametype->lua, -1);
            error_push("lua error: %s", err);
            return false;
        }
        return true;
    }

    // What do we even have?
    error_push("Gametype initialization is not a function or nil.");
    lua_pop(gametype->lua, 1);
    return false;
}
