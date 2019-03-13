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

#include "ruleset.h"

#include "define.h"

#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "physfs.h"

#include "error.h"
#include "frontend.h"
#include "menu.h"
#include "script.h"
#include "vfs.h"

/**
 * Allocate a new ruleset.
 */
ruleset_t* ruleset_new(lua_State* L, const char* name) {
    char* filename = NULL;
    vfile_t* file = NULL;
    ruleset_t* ruleset = NULL;

    // Create the structure to actually hold our ruleset.
    if ((ruleset = calloc(1, sizeof(ruleset_t))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Make sure our Lua state is set, so cleanup works properly.
    ruleset->lua = L;
    ruleset->state_frame_ref = LUA_NOREF;
    ruleset->init_ref = LUA_NOREF;
    ruleset->env_ref = LUA_NOREF;

    int ok = asprintf(&filename, "ruleset/%s/main.lua", name);
    if (ok < 0) {
        error_push_allocerr();
        goto fail;
    }

    // Load the file with our ruleset in it.
    if ((file = vfs_vfile_new(filename)) == NULL) {
        error_push("Could not find ruleset %s.", name);
        goto fail;
    }

    // Load our file into the Lua state.
    if (luaL_loadbufferx(L, (char*)file->data, file->size, filename, "t") != LUA_OK) {
        error_push("%s", lua_tostring(L, -1));
        goto fail;
    }

    // Free resources that we don't need anymore.
    free(filename);
    filename = NULL;
    vfs_vfile_delete(file);
    file = NULL;

    // Create a restricted ruleset environment and push a ref to it into
    // the registry.
    lua_createtable(L, 0, 0);
    lua_pushvalue(L, -1);
    if ((ruleset->env_ref = luaL_ref(L, LUA_REGISTRYINDEX)) == LUA_REFNIL) { // pop env table dupe
        error_push_allocerr();
        goto fail;
    }

    // Set up links to the proper modules and globals.
    const char* modules[] = {
        "mino_ruleset", "mino_board", "mino_piece", "mino_audio", "mino_random",
        "mino_next", "mino_input", "mino_state"
    };
    lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
    for (size_t i = 0;i < ARRAY_LEN(modules);i++) {
        lua_getfield(L, -1, modules[i]);
        lua_setfield(L, -3, modules[i]);
    }
    lua_pop(L, 1);

    const char* globals[] = {
        "ipairs", "next", "pairs", "print", "require", "tostring", "_VERSION"
    };
    for (size_t i = 0;i < ARRAY_LEN(globals);i++) {
        lua_getglobal(L, globals[i]);
        lua_setfield(L, -2, globals[i]);
    }

    // Setup the global "package.loaded" table we use for packages.
    lua_createtable(L, 0, 1); // package table
    lua_createtable(L, 0, 0); // loaded table
    lua_setfield(L, -2, "loaded"); // pops loaded table
    lua_setfield(L, -2, "package"); // pops package table

    // Set our environment context, both in the registry and as our _ENV.
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "env"); // pop env copy
    lua_setupvalue(L, -2, 1); // pop env

    // We now have the ruleset on the top of the stack.  Call it!
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        error_push("lua error: %s", lua_tostring(L, -1));
        goto fail;
    }

    // We should have a table at the top of the stack that contains the
    // functions that implement the ruleset.
    if (lua_type(L, -1) != LUA_TTABLE) {
        error_push("Could not find module table in ruleset %s.", name);
        goto fail;
    }

    // Check for a table key that contains the state_frame function.
    int type = lua_getfield(L, -1, "state_frame");
    if (type == LUA_TNIL) {
        error_push("Could not find function \"state_frame\" in module table in ruleset %s.", name);
        goto fail;
    } else if (type != LUA_TFUNCTION) {
        error_push("\"state_frame\" is not a function in ruleset %s.", name);
        goto fail;
    }

    // Create a reference to the state_frame function.
    if ((ruleset->state_frame_ref = luaL_ref(L, LUA_REGISTRYINDEX)) == LUA_REFNIL) {
        error_push_allocerr();
        goto fail;
    }

    // Check for a table key that contains the initialization function.
    type = lua_getfield(L, -1, "init");
    if (type == LUA_TNIL) {
        error_push("Could not find table \"init\" in module table in ruleset %s.", name);
        goto fail;
    } else if (type != LUA_TFUNCTION) {
        error_push("\"init\" is not a function in ruleset %s.", name);
        goto fail;
    }

    // Create a reference to the state table.
    if ((ruleset->init_ref = luaL_ref(L, LUA_REGISTRYINDEX)) == LUA_REFNIL) {
        error_push_allocerr();
        goto fail;
    }

    // Load our pieces.
    lua_pushstring(L, "pieces");
    type = lua_gettable(L, -2);
    if (type == LUA_TNIL) {
        error_push("Could not find table \"pieces\" in module table in ruleset %s.", name);
        goto fail;
    } else if (type != LUA_TTABLE) {
        error_push("\"pieces\" is not a table in ruleset %s.", name);
        goto fail;
    }

    // Iterate through the piece array.
    if ((ruleset->pieces = piece_configs_new(L)) == NULL) {
        error_push("Error parsing pieces in ruleset %s: %s", name, lua_tostring(L, -1));
        goto fail;
    }

    // We should only have the ruleset on the stack.  Always finish your
    // Lua meddling with a clean stack.
    if (lua_gettop(L) != 1) {
        error_push("Lua stack not cleaned up.");
        goto fail;
    }
    lua_pop(L, 1);

    // Copy our ruleset name - has the potential to fail.
    ruleset->name = strdup(name);
    if (ruleset->name == NULL) {
        error_push_allocerr();
        goto fail;
    }

    return ruleset;

fail:
    free(filename);
    vfs_vfile_delete(file);
    ruleset_delete(ruleset);

    return NULL;
}

/**
 * Free a ruleset.
 */
void ruleset_delete(ruleset_t* ruleset) {
    if (ruleset == NULL) {
        return;
    }

    free(ruleset->name);
    luaL_unref(ruleset->lua, LUA_REGISTRYINDEX, ruleset->env_ref);
    luaL_unref(ruleset->lua, LUA_REGISTRYINDEX, ruleset->state_frame_ref);
    luaL_unref(ruleset->lua, LUA_REGISTRYINDEX, ruleset->init_ref);
    piece_configs_delete(ruleset->pieces);

    free(ruleset);
}

/**
 * Get a list of gametypes that can be played in a given ruleset
 * 
 * This function allocates a menulist.  It's up to the caller to free it.
 */
menulist_t* ruleset_get_gametypes(ruleset_t* ruleset) {
    char* gametypepath = NULL;
    menulist_t* list = NULL;
    char** gametypes = NULL;

    int ok = asprintf(&gametypepath, "gametype/%s", ruleset->name);
    if (ok < 0) {
        error_push_allocerr();
        goto fail;
    }

    list = menulist_new();
    if (list == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Iterate over all of the ruleset gametype directories
    gametypes = PHYSFS_enumerateFiles(gametypepath);
    free(gametypepath);
    gametypepath = NULL;
    if (gametypes == NULL) {
        // Allocation error
        goto fail;
    }
    for (char** i = gametypes;*i != NULL;i++) {
        // Try and find a file called manifest.cfg in every directory
        char* mainpath = NULL;
        ok = asprintf(&mainpath, "gametype/%s/%s/manifest.cfg", ruleset->name, *i);
        if (ok < 0) {
            // Allocation error.
            goto fail;
        }

        vfile_t* file = vfs_vfile_new(mainpath);
        free(mainpath);
        if (file == NULL) {
            // TODO: A warning message would be nice.
            continue;
        }

        int top = lua_gettop(ruleset->lua);

        // Load the config file - pushes the data or an error to the stack.
        if (!script_load_config(ruleset->lua, file)) {
            error_push("%s", lua_tostring(ruleset->lua, -1));
            vfs_vfile_delete(file);
            lua_settop(ruleset->lua, top);
            continue;
        }
        vfs_vfile_delete(file);

        // Set our menu parameters using values from Lua.
        menulist_params_t params;
        params.value = *i;
        lua_getfield(ruleset->lua, -1, "label");
        params.label = lua_tostring(ruleset->lua, -1);
        lua_getfield(ruleset->lua, -2, "help");
        params.help = lua_tostring(ruleset->lua, -1);
        lua_getfield(ruleset->lua, -3, "position");
        params.position = lua_tointeger(ruleset->lua, -1);

        // Push the menu parameters into our menu
        menulist_push(list, &params);

        // Clean up the stack
        lua_settop(ruleset->lua, top);
    }
    PHYSFS_freeList(gametypes);
    gametypes = NULL;

    return list;

fail:
    if (gametypepath != NULL) {
        free(gametypepath);
        gametypepath = NULL;
    }

    if (list != NULL) {
        menulist_delete(list);
        list = NULL;
    }

    if (gametypes != NULL) {
        PHYSFS_freeList(gametypes);
        gametypes = NULL;
    }

    return NULL;
}

/**
 * Run our initialization functions for a new game
 */
bool ruleset_initialize(ruleset_t* ruleset, state_t* state) {
    // Ensure our state is in the registry
    lua_pushlightuserdata(ruleset->lua, state);
    lua_setfield(ruleset->lua, LUA_REGISTRYINDEX, "state");

    // Get our initialization function out of the registry
    int type = lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->init_ref);
    if (type == LUA_TNIL) {
        // No initilization function, we're fine.
        return true;
    } else if (type == LUA_TFUNCTION) {
        // Call the function.
        if (lua_pcall(ruleset->lua, 0, 0, 0) != LUA_OK) {
            const char* err = lua_tostring(ruleset->lua, -1);
            error_push("lua error: %s", err);
            return false;
        }
        return true;
    }

    // What do we even have?
    error_push("Ruleset initialization is not a function or nil.");
    lua_pop(ruleset->lua, 1);
    return false;
}
