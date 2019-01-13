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

#include <stdlib.h>

#include "lauxlib.h"

#include "frontend.h"
#include "ruleset.h"
#include "vfs.h"

// static void ruleset_require(const char* base, const char* name) {
//     (void)base;
//     (void)name;
// }

ruleset_t* ruleset_new(void) {
    // Load the file with our ruleset in it.
    buffer_t* file = vfs_file("ruleset/default/main.lua");
    if (file == NULL) {
        frontend_fatalerror("Could not find ruleset %s", "default");
        return NULL;
    }

    // Create a new Lua state
    lua_State* l = luaL_newstate();
    if (l == NULL) {
        buffer_delete(file);
        return NULL;
    }

    // Load our file into the newly-created Lua state.
    if (luaL_loadbuffer(l, (char*)file->data, file->size, "main") != LUA_OK) {
        buffer_delete(file);
        lua_close(l);
        return NULL;
    }
    buffer_delete(file);

    // We're all set!  Create the structure to actually hold our ruleset.
    ruleset_t* ruleset = calloc(1, sizeof(ruleset_t));
    if (ruleset == NULL) {
        return NULL;
    }

    ruleset->lua = l;

    return ruleset;
}

void ruleset_delete(ruleset_t* ruleset) {
    if (ruleset->lua != NULL) {
        lua_close(ruleset->lua);
        ruleset->lua = NULL;
    }

    free(ruleset);
}

state_result_t ruleset_state_frame(ruleset_t* ruleset, state_t* state,
                                const playerevents_t* playerevents) {
    (void)ruleset;
    (void)state;
    (void)playerevents;

    return STATE_RESULT_OK;
}
