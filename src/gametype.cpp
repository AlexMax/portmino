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

#include "gametype.hpp"

#include <stdlib.h>
#include <string.h>

#include "lua.hpp"

#include "error.hpp"
#include "ruleset.hpp"
#include "script.hpp"
#include "vfs.hpp"

/**
 * Allocate a new gametype
 */
gametype_t* gametype_new(lua_State* L, ruleset_t* ruleset, const char* name) {
    int top = lua_gettop(L);

    char* filename = NULL;
    vfile_t* file = NULL;
    gametype_t* gametype = NULL;

    // Create the structure to actually hold our gametype.
    if ((gametype = calloc(1, sizeof(*gametype))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Make sure our Lua state is set, so cleanup works properly.
    gametype->lua = L;

    // Construct path to gametype configuration.
    int ok = asprintf(&filename, "gametype/%s/%s/gametype.cfg", ruleset->name, name);
    if (ok < 0) {
        error_push_allocerr();
        goto fail;
    }

    // Load the file with our gametype in it.
    if ((file = vfs_vfile_new(filename, 0)) == NULL) {
        error_push("Could not find gametype %s.", name);
        goto fail;
    }

    // Load our file into the Lua state.
    if (script_load_config(L, file) == false) {
        error_push("%s", lua_tostring(L, -1));
        goto fail;
    }

    // Do we have a label in our manifest?
    if (lua_getfield(L, -1, "label") != LUA_TSTRING) {
        error_push("Gametype (%s) label is not string.", name);
        goto fail;
    }

    // Convert our label to a string.
    if ((gametype->label = strdup(lua_tostring(L, -1))) == NULL) {
        error_push_allocerr();
        goto fail;
    }
    lua_pop(L, 1); // pop label

    // Do we have a help string in our manifest?
    if (lua_getfield(L, -1, "help") != LUA_TSTRING) {
        error_push("Gametype (%s) help is not string.", name);
        goto fail;
    }

    // Convert our help string to a string.
    if ((gametype->help = strdup(lua_tostring(L, -1))) == NULL) {
        error_push_allocerr();
        goto fail;
    }
    lua_pop(L, 1); // pop help string

    // Free resources that we don't need anymore.
    free(filename);
    filename = NULL;
    vfs_vfile_delete(file);
    file = NULL;

    // Copy our gametype name
    gametype->name = strdup(name);
    if (gametype->name == NULL) {
        error_push_allocerr();
        goto fail;
    }

    return gametype;

fail:
    free(filename);
    vfs_vfile_delete(file);
    gametype_delete(gametype);
    lua_settop(L, top);

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
    gametype->name = NULL;
    free(gametype->label);
    gametype->label = NULL;
    free(gametype->help);
    gametype->help = NULL;

    free(gametype);
}
