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

#include "lua.h"
#include "lauxlib.h"

#include "frontend.h"
#include "ruleset.h"
#include "vfs.h"

/**
 * Allocate a new ruleset.
 */
ruleset_t* ruleset_new(void) {
    // Load the file with our ruleset in it.
    buffer_t* file = vfs_file("ruleset/default/main.lua");
    if (file == NULL) {
        frontend_fatalerror("Could not find ruleset %s", "default");
        return NULL;
    }

    // Create a new Lua state
    lua_State* L = luaL_newstate();
    if (L == NULL) {
        buffer_delete(file);
        return NULL;
    }

    // Load our file into the newly-created Lua state.
    if (luaL_loadbuffer(L, (char*)file->data, file->size, "main") != LUA_OK) {
        buffer_delete(file);
        lua_close(L);
        frontend_fatalerror("Could not load ruleset %s", "default");
        return NULL;
    }
    buffer_delete(file);

    // We now have the ruleset on the top of the stack.  Call it!
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        lua_close(L);
        frontend_fatalerror("Could not execute ruleset %s", "default");
        return NULL;
    }

    // We should have a table at the top of the stack that contains the
    // functions that implement the ruleset.
    if (lua_type(L, -1) != LUA_TTABLE) {
        lua_close(L);
        frontend_fatalerror("Could not find module table in ruleset %s", "default");
        return NULL;
    }

    // Check for a table key that contains the state_frame function.
    lua_pushstring(L, "state_frame");
    lua_gettable(L, -2);
    if (!lua_isfunction(L, -1)) {
        lua_close(L);
        frontend_fatalerror("Could not find function \"state_frame\" in module table in ruleset %s", "default");
        return NULL;
    }

    // We should have a table at the top of the stack that contains the
    // functions that implement the ruleset.
    if (lua_type(L, -1) != LUA_TFUNCTION) {
        lua_close(L);
        frontend_fatalerror("\"state_frame\" is not a function in ruleset %s", "default");
        return NULL;
    }

    // We're all set!  Create the structure to actually hold our ruleset.
    ruleset_t* ruleset = calloc(1, sizeof(ruleset_t));
    if (ruleset == NULL) {
        return NULL;
    }

    ruleset->lua = L;

    return ruleset;
}

/**
 * Free a ruleset.
 */
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

    // We expect the top of the stack to contain a function that contains
    // our state_frame function.
    if (lua_pcall(ruleset->lua, 0, 0, 0) != LUA_OK) {
        const char* err = lua_tostring(ruleset->lua, -1);
        frontend_fatalerror("lua error: %s", err);
        return STATE_RESULT_ERROR;
    }

    return STATE_RESULT_OK;
}
