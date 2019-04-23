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
    int top = lua_gettop(L);

    char* filename = NULL;
    vfile_t* file = NULL;
    ruleset_t* ruleset = NULL;

    // Create the structure to actually hold our ruleset.
    if ((ruleset = calloc(1, sizeof(*ruleset))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Make sure our Lua state is set, so cleanup works properly.
    ruleset->lua = L;

    // Construct path to ruleset configuration.
    int ok = asprintf(&filename, "ruleset/%s/ruleset.cfg", name);
    if (ok < 0) {
        error_push_allocerr();
        goto fail;
    }

    // Load the file with our ruleset in it.
    if ((file = vfs_vfile_new(filename, 0)) == NULL) {
        error_push("Could not find ruleset %s.", name);
        goto fail;
    }

    // Load our file into the Lua state.
    if (script_load_config(L, file) == false) {
        error_push("%s", lua_tostring(L, -1));
        goto fail;
    }

    // Do we have a label in our manifest?
    if (lua_getfield(L, -1, "label") != LUA_TSTRING) {
        error_push("Ruleset (%s) label is not string.", name);
        goto fail;
    }

    // Convert our label to a string.
    if ((ruleset->label = strdup(lua_tostring(L, -1))) == NULL) {
        error_push_allocerr();
        goto fail;
    }
    lua_pop(L, 1); // pop label

    // Do we have a help string in our manifest?
    if (lua_getfield(L, -1, "help") != LUA_TSTRING) {
        error_push("Ruleset (%s) help is not string.", name);
        goto fail;
    }

    // Convert our help string to a string.
    if ((ruleset->help = strdup(lua_tostring(L, -1))) == NULL) {
        error_push_allocerr();
        goto fail;
    }
    lua_pop(L, 1); // pop help string

    // Free resources that we don't need anymore.
    free(filename);
    filename = NULL;
    vfs_vfile_delete(file);
    file = NULL;

    // Copy our ruleset name
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
    lua_settop(L, top);

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
    free(ruleset->label);
    ruleset->label = NULL;
    free(ruleset->help);
    ruleset->help = NULL;

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
        // Try and find a file called gametype.cfg in every directory
        char* mainpath = NULL;
        ok = asprintf(&mainpath, "gametype/%s/%s/gametype.cfg", ruleset->name, *i);
        if (ok < 0) {
            // Allocation error.
            goto fail;
        }

        vfile_t* file = vfs_vfile_new(mainpath, 0);
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
