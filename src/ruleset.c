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
    buffer_t* file = NULL;
    piece_configs_t* piece_configs = NULL;
    ruleset_t* ruleset = NULL;

    int ok = asprintf(&filename, "ruleset/%s/main.lua", name);
    if (ok < 0) {
        error_push_allocerr();
        return NULL;
    }

    // Load the file with our ruleset in it.
    if ((file = vfs_file(filename)) == NULL) {
        error_push("Could not find ruleset %s.", name);
        goto fail;
    }

    // Load our file into the newly-created Lua state.
    if (luaL_loadbufferx(L, (char*)file->data, file->size, filename, "t") != LUA_OK) {
        error_push("%s", lua_tostring(L, -1));
        goto fail;
    }

    // Free resources that we don't need anymore.
    free(filename);
    filename = NULL;
    buffer_delete(file);
    file = NULL;

    // Create a restricted ruleset environment and push a ref to it into
    // the registry.
    lua_createtable(L, 0, 0);
    lua_pushvalue(L, -1);
    int env_ref = luaL_ref(L, LUA_REGISTRYINDEX); // pop env table dupe
    if (env_ref == LUA_REFNIL) {
        error_push_allocerr();
        goto fail;
    }

    // Set up links to the proper modules and globals.
    const char* modules[] = {
        "mino_ruleset", "mino_board", "mino_piece", "mino_audio", "mino_random",
        "mino_next", "mino_event"
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
        error_push("%s", lua_tostring(L, -1));
        goto fail;
    }

    // We should have a table at the top of the stack that contains the
    // functions that implement the ruleset.
    if (lua_type(L, -1) != LUA_TTABLE) {
        error_push("Could not find module table in ruleset %s.", name);
        goto fail;
    }

    // Check for a table key that contains the state_frame function.
    lua_pushstring(L, "state_frame");
    int type = lua_gettable(L, -2);
    if (type == LUA_TNIL) {
        error_push("Could not find function \"state_frame\" in module table in ruleset %s.", name);
        goto fail;
    } else if (type != LUA_TFUNCTION) {
        error_push("\"state_frame\" is not a function in ruleset %s.", name);
        goto fail;
    }

    // Create a reference to the state_frame function.
    int state_frame_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (state_frame_ref == LUA_REFNIL) {
        error_push_allocerr();
        goto fail;
    }

    // Check for a table key that contains the state table.
    lua_pushstring(L, "state");
    type = lua_gettable(L, -2);
    if (type == LUA_TNIL) {
        error_push("Could not find table \"state\" in module table in ruleset %s.", name);
        goto fail;
    } else if (type != LUA_TTABLE) {
        error_push("\"state\" is not a table in ruleset %s.", name);
        goto fail;
    }

    // Create a reference to the state table.
    int state_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (state_ref == LUA_REFNIL) {
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
    if ((piece_configs = piece_configs_new(L)) == NULL) {
        error_push("Error parsing pieces in ruleset %s: %s", name, lua_tostring(L, -1));
        goto fail;
    }

    // Check for a table key that contains the next_piece function.
    lua_pushstring(L, "next_piece");
    type = lua_gettable(L, -2);
    if (type == LUA_TNIL) {
        error_push("Could not find function \"next_piece\" in module table in ruleset %s.", name);
        goto fail;
    } else if (type != LUA_TFUNCTION) {
        error_push("\"next_piece\" is not a function in ruleset %s.", name);
        goto fail;
    }

    // Create a reference to the next_piece function.
    int next_piece_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (next_piece_ref == LUA_REFNIL) {
        error_push_allocerr();
        goto fail;
    }

    // We should only have the ruleset on the stack.  Always finish your
    // Lua meddling with a clean stack.
    if (lua_gettop(L) != 1) {
        error_push("Lua stack not cleaned up.");
        goto fail;
    }
    lua_pop(L, 1);

    // We're all set!  Create the structure to actually hold our ruleset.
    if ((ruleset = calloc(1, sizeof(ruleset_t))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Copy our ruleset name - has the potential to fail.
    ruleset->name = strdup(name);
    if (ruleset->name == NULL) {
        error_push_allocerr();
        goto fail;
    }

    ruleset->lua = L;
    ruleset->state_frame_ref = state_frame_ref;
    ruleset->state_ref = state_ref;
    ruleset->pieces = piece_configs;
    ruleset->next_piece_ref = next_piece_ref;
    ruleset->env_ref = env_ref;

    return ruleset;

fail:
    free(filename);
    buffer_delete(file);
    piece_configs_delete(piece_configs);
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
    ruleset->name = NULL;

    piece_configs_delete(ruleset->pieces);
    ruleset->pieces = NULL;

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
        // Allocation error
        goto fail;
    }

    list = menulist_new();
    if (list == NULL) {
        // Allocation error
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

        buffer_t* file = vfs_file(mainpath);
        free(mainpath);
        if (file == NULL) {
            // TODO: A warning message would be nice.
            continue;
        }

        int top = lua_gettop(ruleset->lua);

        // Load the config file - pushes the data or an error to the stack.
        if (!script_load_config(ruleset->lua, file, *i)) {
            error_push("%s", lua_tostring(ruleset->lua, -1));
            buffer_delete(file);
            lua_settop(ruleset->lua, top);
            continue;
        }
        buffer_delete(file);

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

ruleset_result_t ruleset_frame(ruleset_t* ruleset, state_t* state,
                               const playerevents_t* playerevents) {
    // Push our context into the registry so we can get at them from C
    // functions called from inside Lua.
    lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->env_ref);
    lua_setfield(ruleset->lua, LUA_REGISTRYINDEX, "env");
    lua_pushlightuserdata(ruleset->lua, ruleset);
    lua_setfield(ruleset->lua, LUA_REGISTRYINDEX, "ruleset");
    lua_pushlightuserdata(ruleset->lua, state);
    lua_setfield(ruleset->lua, LUA_REGISTRYINDEX, "state");
    lua_pushlightuserdata(ruleset->lua, (void*)playerevents);
    lua_setfield(ruleset->lua, LUA_REGISTRYINDEX, "playerevents");

    // Use our references to grab the state_frame function and its environment
    lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->state_frame_ref);
    lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->env_ref);

    // Setup the environment
    lua_setupvalue(ruleset->lua, -2, 1);

    // Run the state_frame function.
    if (lua_pcall(ruleset->lua, 0, 1, 0) != LUA_OK) {
        const char* err = lua_tostring(ruleset->lua, -1);
        frontend_fatalerror("lua error: %s", err);
        return RULESET_RESULT_ERROR;
    }

    // Handle our result
    ruleset_result_t result = lua_tointeger(ruleset->lua, -1);
    lua_pop(ruleset->lua, 1); // pop result

    return result;
}

/**
 * Return a "next" piece by calling into our Lua function to get it
 */
const piece_config_t* ruleset_next_piece(ruleset_t* ruleset, next_t* next) {
    // Use our reference to grab the next_piece function.
    lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->next_piece_ref);

    lua_pushstring(ruleset->lua, "ruleset");
    lua_pushlightuserdata(ruleset->lua, (void*)ruleset);
    lua_settable(ruleset->lua, LUA_REGISTRYINDEX);

    // Pass a 1-indexed board ID to the next piece function, so we know which
    // board we're spawning the piece on.
    lua_pushinteger(ruleset->lua, next->id + 1);

    // Call it, friendo
    if (lua_pcall(ruleset->lua, 1, 1, 0) != LUA_OK) {
        const char* err = lua_tostring(ruleset->lua, -1);
        frontend_fatalerror("lua error: %s", err);
        return NULL;
    }

    // Return either our config pointer or NULL
    return lua_touserdata(ruleset->lua, -1);
}
